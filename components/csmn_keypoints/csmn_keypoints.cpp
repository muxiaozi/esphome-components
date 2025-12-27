#include "csmn_keypoints.h"
#include "utils.h"
#include <esphome/core/log.h>
#include <string.h>
#include <esphome/core/application.h>

namespace esphome {
namespace csmn_keypoints {

static const char *const TAG = "csmn_keypoints";

static const uint32_t MIN_RECORD_DURATION_SECONDS = 3;

CsmnKeyPoints::CsmnKeyPoints() {
  this->sdcard_ = std::make_unique<Sdcard>();
  this->uploader_ = std::make_unique<Uploader>();
  this->api_ = std::make_unique<Api>();

  this->uploader_->set_api(this->api_.get());
  this->uploader_->set_sdcard(this->sdcard_.get());

  this->api_->set_sdcard(this->sdcard_.get());
}

void CsmnKeyPoints::setup() {
  if (!this->sdcard_->setup(this->cmd_pin_->get_pin(), this->clk_pin_->get_pin(), this->d0_pin_->get_pin())) {
    ESP_LOGE(TAG, "SD card setup failed");
    this->mark_failed(LOG_STR("SD card setup failed"));
    return;
  }

  if (!global_csmn_keypoints_config->setup()) {
    ESP_LOGE(TAG, "Config setup failed");
    this->mark_failed(LOG_STR("Config setup failed"));
    return;
  }

  if (!this->uploader_->setup()) {
    ESP_LOGE(TAG, "Uploader setup failed");
    this->mark_failed(LOG_STR("Uploader setup failed"));
    return;
  }

  this->microphone_->add_data_callback([this](const std::vector<uint8_t> &data) {
    if (!this->recording_) {
      return;
    }

    this->sdcard_->write_data(data.data(), data.size());
  });
}

void CsmnKeyPoints::loop() {}

void CsmnKeyPoints::dump_config() {
  ESP_LOGCONFIG(TAG, "CsmnKeyPoints:");
  ESP_LOGCONFIG(TAG, "  UUID: %s", this->uuid_.c_str());
  ESP_LOGCONFIG(TAG, "  Http Base URL: %s", this->base_url_.c_str());
  ESP_LOGCONFIG(TAG, "  SD Card:");
  ESP_LOGCONFIG(TAG, "    CMD Pin: %u", this->cmd_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "    CLK Pin: %u", this->clk_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "    D0 Pin: %u", this->d0_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  Record Status:");
  ESP_LOGCONFIG(TAG, "    Create Index: %u", global_csmn_keypoints_config->get_create_index());
  ESP_LOGCONFIG(TAG, "    Upload Index: %u", global_csmn_keypoints_config->get_upload_index());
}

void CsmnKeyPoints::start() {
  if (this->recording_) {
    ESP_LOGW(TAG, "Already recording");
    return;
  }

  uint32_t record_id = global_csmn_keypoints_config->new_record();
  if (!this->sdcard_->open_wav(record_id)) {
    global_csmn_keypoints_config->set_record_flag(record_id, FLAG_SKIP_UPLOAD);
    ESP_LOGE(TAG, "Failed to open WAV file for record ID %u", record_id);
    return;
  }

  ESP_LOGI(TAG, "Starting recording: %u", record_id);

  auto stream_info = this->microphone_->get_audio_stream_info();
  this->sdcard_->write_header(stream_info.get_sample_rate(), stream_info.get_bits_per_sample(),
                              stream_info.get_channels());

  this->recording_ = true;
  global_csmn_keypoints_config->set_recording(true);
  this->microphone_->start();
}

void CsmnKeyPoints::stop() {
  if (!this->recording_) {
    ESP_LOGW(TAG, "Not recording");
    return;
  }

  ESP_LOGI(TAG, "Stopping recording");

  this->recording_ = false;
  global_csmn_keypoints_config->set_recording(false);
  this->microphone_->stop();

  auto record_id = global_csmn_keypoints_config->get_create_index();
  auto stream_info = this->microphone_->get_audio_stream_info();
  uint32_t duration_seconds = stream_info.bytes_to_ms(this->sdcard_->get_sample_size()) / 1000;
  if (duration_seconds < MIN_RECORD_DURATION_SECONDS) {
    ESP_LOGW(TAG, "Recording too short (%u seconds), discarding", duration_seconds);
    this->sdcard_->finish(stream_info.get_sample_rate(), stream_info.get_bits_per_sample(), stream_info.get_channels());
    global_csmn_keypoints_config->set_record_flag(record_id, FLAG_SKIP_UPLOAD);
    return;
  }

  this->sdcard_->finish(stream_info.get_sample_rate(), stream_info.get_bits_per_sample(), stream_info.get_channels());
  std::string file_path = this->sdcard_->get_filepath(record_id);
  auto file_size = this->sdcard_->get_file_size();
  uint16_t file_crc16 = 0;  // 需要开一个线程计算CRC16，暂时设为0
  global_csmn_keypoints_config->finish_record(duration_seconds, file_size, file_crc16);
}

}  // namespace csmn_keypoints
}  // namespace esphome

#include "config.h"

#include <esphome/core/log.h>
#include <esphome/core/helpers.h>
#include <esp_timer.h>
#include "utils.h"

namespace esphome {
namespace csmn_keypoints {

static const char *TAG = "csmn_keypoints::config";
static char current_section_[8]{};

bool Config::setup() {
  uint32_t common_hash = fnv1_hash(std::string("csmn_keypoints_common"));
  this->common_pref_ = global_preferences->make_preference<CommonConfig>(common_hash, true);

  if (!this->common_pref_.load(&this->common_config_)) {
    ESP_LOGD(TAG, "No existing common config, initializing");
    this->common_config_.create_index = 0;
    this->common_config_.upload_index = 0;
    this->common_pref_.save(&this->common_config_);
  } else {
    ESP_LOGD(TAG, "Loaded common config: create_index=%u, upload_index=%u", this->common_config_.create_index,
             this->common_config_.upload_index);
  }

  return true;
}

uint32_t Config::new_record() {
  // 更新创建索引
  this->common_config_.create_index += 1;
  this->common_pref_.save(&this->common_config_);

  uint32_t hash = fnv1_hash(std::string("csmn_keypoints_record_") + std::to_string(this->common_config_.create_index));
  this->current_record_pref_ = global_preferences->make_preference<RecordConfig>(hash, true);
  RecordConfig record_config{};
  record_config.start_time = esp_timer_get_time() / 1000;  // RTC时间
  this->current_record_pref_.save(&record_config);

  global_preferences->sync();

  return this->common_config_.create_index;
}

std::optional<uint32_t> Config::next_upload_index() {
  uint32_t upload_index = this->common_config_.upload_index;
  uint32_t create_limit = this->common_config_.create_index;
  if (create_limit == 0) {
    return std::nullopt;
  }

  if (upload_index == 0) {
    upload_index = 1;
  }

  while (true) {
    RecordConfig config{};
    ESP_LOGD(TAG, "Checking record ID %u for upload", upload_index);
    if (get_record_config(upload_index, &config)) {
      if (!check_flag(config.flags, FLAG_RECORDED)) {
        // 等待录音完成
        if (this->is_recording()) {
          ESP_LOGD(TAG, "Currently recording, cannot upload record ID %u yet", upload_index);
          return std::nullopt;
        }
      } else {
        // 未上传成功且不跳过上传
        if (!check_flag(config.flags, FLAG_UPLOADED) && !check_flag(config.flags, FLAG_SKIP_UPLOAD)) {
          ESP_LOGD(TAG, "Record ID %u ready to upload", upload_index);
          return upload_index;
        }
      }
    }

    if (upload_index < create_limit) {
      ESP_LOGD(TAG, "Record ID %u already uploaded or skipped, checking next", upload_index);
      upload_index++;
      this->common_config_.upload_index = upload_index;
      this->common_pref_.save(&this->common_config_);
      global_preferences->sync();
    } else {
      return std::nullopt;
    }
  }
}

void Config::finish_record(uint32_t duration_seconds, uint32_t size_bytes, uint16_t file_crc16) {
  RecordConfig record_config{};
  this->current_record_pref_.load(&record_config);
  record_config.duration_seconds = duration_seconds;
  record_config.size_bytes = size_bytes;
  record_config.flags |= FLAG_RECORDED;
  record_config.crc16 = file_crc16;
  this->current_record_pref_.save(&record_config);

  global_preferences->sync();
}

bool Config::get_record_config(uint32_t record_id, RecordConfig *config) {
  uint32_t hash = fnv1_hash(std::string("csmn_keypoints_record_") + std::to_string(record_id));
  ESPPreferenceObject record_pref = global_preferences->make_preference<RecordConfig>(hash, true);
  if (!record_pref.load(config)) {
    ESP_LOGW(TAG, "No record config found for record ID %u", record_id);
    return false;
  }
  return true;
}

void Config::set_record_flag(uint32_t record_id, uint8_t flag) {
  uint32_t hash = fnv1_hash(std::string("csmn_keypoints_record_") + std::to_string(record_id));
  ESPPreferenceObject record_pref = global_preferences->make_preference<RecordConfig>(hash, true);
  RecordConfig config{};
  if (!record_pref.load(&config)) {
    ESP_LOGW(TAG, "No record config found for record ID %u", record_id);
    return;
  }
  config.flags |= flag;
  record_pref.save(&config);

  global_preferences->sync();
}

Config *global_csmn_keypoints_config = new Config();

}  // namespace csmn_keypoints
}  // namespace esphome
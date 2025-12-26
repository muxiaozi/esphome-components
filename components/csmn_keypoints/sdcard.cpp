#include "sdcard.h"

#include <esphome/core/log.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_timer.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include "format_wav.h"

#define MOUNT_POINT "/sdcard"

namespace esphome {
namespace csmn_keypoints {

const char *TAG = "csmn_keypoints::sdcard";

void dump_sd_info(sdmmc_card_t *card) {
  char content[256]{};
  FILE *f = fmemopen(content, sizeof(content), "w");
  sdmmc_card_print_info(f, card);
  fclose(f);
  ESP_LOGI(TAG, "%s", content);
}

bool Sdcard::setup(uint8_t miso_pin, uint8_t mosi_pin, uint8_t clk_pin, uint8_t cs_pin) {
  esp_err_t ret;

  ESP_LOGI(TAG, "Initializing SD card");

  ESP_LOGI(TAG, "Using SPI peripheral");
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  // host.max_freq_khz = 5000;

  spi_bus_config_t bus_cfg = {
      .mosi_io_num = mosi_pin,
      .miso_io_num = miso_pin,
      .sclk_io_num = clk_pin,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };

  ret = spi_bus_initialize(static_cast<spi_host_device_t>(host.slot), &bus_cfg, SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize bus.");
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(100));

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = static_cast<gpio_num_t>(cs_pin);
  slot_config.host_id = static_cast<spi_host_device_t>(host.slot);

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};
  const char mount_point[] = MOUNT_POINT;
  sdmmc_card_t *card;

  ESP_LOGI(TAG, "Mounting filesystem");
  ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(
          TAG,
          "Failed to mount filesystem. "
          "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    } else {
      ESP_LOGE(TAG,
               "Failed to initialize the card (%s). "
               "Make sure SD card lines have pull-up resistors in place.",
               esp_err_to_name(ret));
    }

    return false;
  }
  ESP_LOGI(TAG, "Filesystem mounted");

  dump_sd_info(card);

  return true;
}

bool Sdcard::open_wav(uint32_t record_id) {
  std::string filepath = this->get_filepath(record_id);
  this->file_ = fopen(filepath.c_str(), "w");
  this->bytes_written_ = 0;
  return this->file_ != nullptr;
}

void Sdcard::write_header(uint32_t sample_rate, uint16_t bits_per_sample, uint16_t channels, uint32_t sample_size) {
  if (this->file_) {
    const wav_header_t header = WAV_HEADER_PCM_DEFAULT(sample_size, bits_per_sample, sample_rate, channels);
    fwrite(&header, sizeof(wav_header_t), 1, this->file_);
  }
}

void Sdcard::write_data(const void *data, size_t size) {
  if (this->file_) {
    size_t written = 0;
    while (written < size) {
      size_t chunk_size = size - written;
      if (chunk_size > 4096) {
        chunk_size = 4096;
      }
      size_t ret = fwrite(static_cast<const uint8_t *>(data) + written, 1, chunk_size, this->file_);
      if (ret == 0) {
        ESP_LOGE(TAG, "Failed to write data to SD card");
        break;
      }
      written += ret;
    }
    this->bytes_written_ += written;
  }
}

void Sdcard::finish(uint32_t sample_rate, uint16_t bits_per_sample, uint16_t channels) {
  if (this->file_) {
    // 更新文件头中的数据长度
    fseek(this->file_, 0, SEEK_SET);
    this->write_header(sample_rate, bits_per_sample, channels, this->bytes_written_);
    fclose(this->file_);
    this->file_ = nullptr;
  }
}

size_t Sdcard::get_file_size() const { return this->bytes_written_ + sizeof(wav_header_t); }

std::string Sdcard::get_filepath(uint32_t record_id) const {
  char filepath[32];
  snprintf(filepath, sizeof(filepath), "%s/%u.wav", MOUNT_POINT, record_id);
  return std::string(filepath);
}

}  // namespace csmn_keypoints
}  // namespace esphome
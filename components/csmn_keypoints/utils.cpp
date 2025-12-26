#include "utils.h"

#include <esphome/core/application.h>
#include <esphome/core/helpers.h>
#include <string>
#include <esp_system.h>
#include <esp_flash.h>
#include <esp_idf_version.h>
#include <esp_mac.h>
#include <sys/stat.h>

namespace esphome {
namespace csmn_keypoints {

static const char *TAG = "csmn_keypoints::utils";

std::string get_name() { return App.get_friendly_name(); }

std::string get_wifi_mac() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char mac_str[18];
  snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return std::string(mac_str);
}

uint32_t get_flash_size_bytes() {
  uint32_t flash_size = 0;
  esp_flash_get_size(nullptr, &flash_size);
  return flash_size;
}

uint32_t get_total_heap_size_bytes() { return esp_get_free_heap_size(); }

std::string get_chip_id() { return std::string(ESPHOME_VARIANT); }

std::string get_reset_reason() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_UNKNOWN:
      return "UNKNOWN";
    case ESP_RST_POWERON:
      return "POWERON";
    case ESP_RST_EXT:
      return "EXTERNAL_PIN";
    case ESP_RST_SW:
      return "SOFTWARE_RESET";
    case ESP_RST_PANIC:
      return "PANIC_RESET";
    case ESP_RST_INT_WDT:
      return "INTERRUPT_WDT_RESET";
    case ESP_RST_TASK_WDT:
      return "TASK_WDT_RESET";
    case ESP_RST_WDT:
      return "WDT_RESET";
    case ESP_RST_DEEPSLEEP:
      return "DEEP_SLEEP_RESET";
    case ESP_RST_BROWNOUT:
      return "BROWNOUT_RESET";
    case ESP_RST_SDIO:
      return "SDIO_RESET";
    case ESP_RST_USB:
      return "USB_RESET";
    case ESP_RST_JTAG:
      return "JTAG_RESET";
    case ESP_RST_EFUSE:
      return "EFUSE_RESET";
    case ESP_RST_PWR_GLITCH:
      return "POWER_GLITCH_RESET";
    case ESP_RST_CPU_LOCKUP:
      return "CPU_LOCKUP_RESET";
    default:
      return "UNKNOWN";
  }
}

std::string get_idf_version() { return esp_get_idf_version(); }

std::string get_firmware_version() { return ESPHOME_PROJECT_VERSION; }

std::string get_manufacturer() {
  size_t dot_pos = std::string(ESPHOME_PROJECT_NAME).find('.');
  if (dot_pos != std::string::npos) {
    return std::string(ESPHOME_PROJECT_NAME).substr(0, dot_pos);
  }
  return ESPHOME_PROJECT_NAME;
}

std::string get_model() {
  size_t dot_pos = std::string(ESPHOME_PROJECT_NAME).find('.');
  if (dot_pos != std::string::npos && dot_pos + 1 < std::string(ESPHOME_PROJECT_NAME).length()) {
    return std::string(ESPHOME_PROJECT_NAME).substr(dot_pos + 1);
  }
  return ESPHOME_PROJECT_NAME;
}

std::string get_description() { return "Csmn Key Points Device"; }

bool file_exists(const std::string &filepath) {
  FILE *f = fopen(filepath.c_str(), "r");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}

uint32_t get_file_size(const std::string &filepath) {
  FILE *f = fopen(filepath.c_str(), "r");
  if (f) {
    fseek(f, 0, SEEK_END);
    uint32_t size = ftell(f);
    fclose(f);
    return size;
  }
  return 0;
}

uint16_t get_file_crc16(const std::string &filepath) {
  FILE *f = fopen(filepath.c_str(), "r");
  if (f) {
    uint8_t buffer[1024];
    uint16_t crc = 0xFFFF;

    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
      crc = crc16(buffer, bytes_read, crc);
    }

    fclose(f);
    return crc;
  }
  return 0;
}

bool check_flag(uint8_t flags, uint8_t flag) {
  return (flags & flag) == flag;
}

}  // namespace csmn_keypoints
}  // namespace esphome
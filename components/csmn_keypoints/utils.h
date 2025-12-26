#pragma once

#include <inttypes.h>
#include <string>

namespace esphome {
namespace csmn_keypoints {

std::string get_name();
std::string get_wifi_mac();
uint32_t get_flash_size_bytes();
uint32_t get_total_heap_size_bytes();
std::string get_chip_id();
std::string get_reset_reason();
std::string get_idf_version();
std::string get_firmware_version();
std::string get_manufacturer();
std::string get_model();
std::string get_description();

bool file_exists(const std::string &filepath);
uint32_t get_file_size(const std::string &filepath);
uint16_t get_file_crc16(const std::string &filepath);

bool check_flag(uint8_t flags, uint8_t flag);

}  // namespace csmn_keypoints
}  // namespace esphome
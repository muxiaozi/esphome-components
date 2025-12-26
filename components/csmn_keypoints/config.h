#pragma once

#include <stdint.h>
#include <optional>
#include <esphome/core/preferences.h>

namespace esphome {
namespace csmn_keypoints {

static const uint8_t FLAG_RECORDED = 0x1;     // 已完成录音
static const uint8_t FLAG_UPLOADED = 0x2;     // 已上传服务器
static const uint8_t FLAG_SKIP_UPLOAD = 0x4;  // 录音文件未找到

struct CommonConfig {
  uint32_t create_index{0};
  uint32_t upload_index{0};
};

struct RecordConfig {
  uint32_t start_time{0};
  uint32_t duration_seconds{0};
  uint32_t size_bytes{0};
  uint8_t flags{0};
  uint16_t crc16{0};
};

class Config {
 public:
  bool setup();

  uint32_t new_record();

  uint32_t get_create_index() const { return this->common_config_.create_index; }
  uint32_t get_upload_index() const { return this->common_config_.upload_index; }
  std::optional<uint32_t> next_upload_index();

  void finish_record(uint32_t duration_seconds, uint32_t size_bytes, uint16_t file_crc16);

  bool get_record_config(uint32_t record_id, RecordConfig *config);

  void set_record_flag(uint32_t record_id, uint8_t flag);

  void set_recording(bool recording) { this->recording_ = recording; }
  bool is_recording() const { return this->recording_; }

 private:
  ESPPreferenceObject common_pref_;
  ESPPreferenceObject current_record_pref_;
  CommonConfig common_config_{};
  bool recording_{false};
};

extern Config *global_csmn_keypoints_config;

}  // namespace csmn_keypoints
}  // namespace esphome
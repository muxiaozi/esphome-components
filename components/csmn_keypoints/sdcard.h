#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string>

namespace esphome {
namespace csmn_keypoints {

class Sdcard {
 public:
  bool setup(uint8_t cmd_pin, uint8_t clk_pin, uint8_t d0_pin);

  bool open_wav(uint32_t record_id);
  void write_header(uint32_t sample_rate, uint16_t bits_per_sample, uint16_t channels, uint32_t sample_size = 0);
  void write_data(const void *data, size_t size);
  void finish(uint32_t sample_rate, uint16_t bits_per_sample, uint16_t channels);

  size_t get_sample_size() const { return this->bytes_written_; }
  size_t get_file_size() const;

  std::string get_filepath(uint32_t record_id) const;

 private:
  FILE *file_{nullptr};
  uint32_t bytes_written_{0};
};

}  // namespace csmn_keypoints
}  // namespace esphome
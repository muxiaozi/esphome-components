#include "ssd1608_dual.h"

#include <algorithm>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::epaper {

using epaper_spi::DELAY_FLAG;
using epaper_spi::MAX_TRANSFER_TIME;

static constexpr const char *const TAG = "epaper";

// SSD1608 register addresses (datasheet Rev 1.2, table 7-1)
static constexpr uint8_t R10_DEEP_SLEEP = 0x10;
static constexpr uint8_t R12_SWRESET = 0x12;
static constexpr uint8_t R20_MASTER_ACTIVATION = 0x20;
static constexpr uint8_t R22_DISPLAY_UPDATE_CONTROL_2 = 0x22;
static constexpr uint8_t R24_WRITE_RAM = 0x24;
static constexpr uint8_t R44_RAM_X_START_END = 0x44;
static constexpr uint8_t R45_RAM_Y_START_END = 0x45;
static constexpr uint8_t R4E_RAM_X_COUNTER = 0x4E;
static constexpr uint8_t R4F_RAM_Y_COUNTER = 0x4F;

void EPaperSSD1608Dual::setup() {
  // Base setup initialises the buffer, the standard pins and the SPI bus.
  epaper_spi::EPaperBase::setup();

  // Both chip-selects are driven directly by this driver (each controller is
  // addressed individually). Start deselected (HIGH).
  this->cs_pin_->setup();
  this->cs_pin_->digital_write(true);
  this->cs1_pin_->setup();
  this->cs1_pin_->digital_write(true);

  ESP_LOGD(TAG, "Setup: %ux%u, buffer %zu bytes, CS/CS1 driven manually", this->width_, this->height_,
           this->buffer_length_);
}

void EPaperSSD1608Dual::loop() {
  const uint32_t now = millis();
  const bool busy = this->busy_pin_ != nullptr && this->busy_pin_->digital_read();
  if (this->state_ != this->last_logged_state_) {
    ESP_LOGD(TAG, "State -> %s (busy=%d)", this->epaper_state_to_string_(), busy);
    this->last_logged_state_ = this->state_;
    this->last_state_change_ = now;
  } else if (this->state_ != epaper_spi::EPaperState::IDLE && now - this->last_state_change_ > 5000) {
    // The base class waits for BUSY without a timeout; surface a stuck state
    // instead of hanging silently.
    ESP_LOGW(TAG, "Stuck in state %s for %u ms (busy=%d)", this->epaper_state_to_string_(),
             (unsigned) (now - this->last_state_change_), busy);
    this->last_state_change_ = now;
  }
  epaper_spi::EPaperBase::loop();
}

bool EPaperSSD1608Dual::reset() {
  if (epaper_spi::EPaperBase::reset()) {
    // The base class releases RES# and returns immediately. Give the chip time
    // to leave reset before talking to it; SSD1608 vendors wait ~100ms here.
    delay(10);
    // Software reset restores the register defaults; it cannot exit deep
    // sleep, which is what the hardware reset above is for.
    this->write_command_both_(R12_SWRESET);
    return true;
  }
  return false;
}

bool EPaperSSD1608Dual::initialise(bool partial) {
  ESP_LOGD(TAG, "Initialise (%s): %u init bytes to both controllers", partial ? "partial" : "full",
           (unsigned) this->init_sequence_length_);
  this->send_init_sequence_both_();
  return true;
}

void EPaperSSD1608Dual::send_init_sequence_both_() {
  // Same wire format as EPaperBase::send_init_sequence_, but every command is
  // sent to both controllers at once.
  size_t index = 0;
  while (index != this->init_sequence_length_) {
    if (this->init_sequence_length_ - index < 2) {
      this->mark_failed(LOG_STR("Malformed init sequence"));
      return;
    }
    const uint8_t cmd = this->init_sequence_[index++];
    if (const uint8_t x = this->init_sequence_[index++]; x == DELAY_FLAG) {
      ESP_LOGV(TAG, "Delay %dms", cmd);
      delay(cmd);
    } else {
      const uint8_t num_args = x & 0x7F;
      if (this->init_sequence_length_ - index < num_args) {
        ESP_LOGE(TAG, "Malformed init sequence, cmd = %X, num_args = %u", cmd, num_args);
        this->mark_failed();
        return;
      }
      this->write_command_(cmd, this->init_sequence_ + index, num_args, true, true);
      index += num_args;
    }
  }
}

void EPaperSSD1608Dual::write_command_(uint8_t command, const uint8_t *data, size_t length, bool use_cs,
                                       bool use_cs1) {
  ESP_LOGV(TAG, "Command: 0x%02X, Length: %u, CS: %d, CS1: %d", command, (unsigned) length, use_cs, use_cs1);
  // Chip-selects are active-low: assert the requested controllers.
  this->cs_pin_->digital_write(!use_cs);
  this->cs1_pin_->digital_write(!use_cs1);
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(command);
  if (length > 0) {
    this->dc_pin_->digital_write(true);
    this->write_array(data, length);
  }
  this->disable();
  this->cs_pin_->digital_write(true);
  this->cs1_pin_->digital_write(true);
}

bool EPaperSSD1608Dual::setup_half_window_(uint8_t half) {
  const uint16_t half_bytes = (this->row_width_ + 1) / 2;
  const uint16_t base = half * half_bytes;
  const uint16_t window_x0 = this->x_low_ / 8;
  const uint16_t window_x1 = this->x_high_ / 8;
  if (window_x0 >= base + half_bytes || window_x1 <= base) {
    return false;  // dirty rectangle does not touch this half
  }
  this->half_x0_ = std::max(window_x0, base) - base;
  this->half_x1_ = std::min(window_x1, (uint16_t) (base + half_bytes)) - base;
  // A flipped chip is mounted rotated 180 degrees: its gate line 0 shows the
  // last buffer row, so the Y window (and the data fed to it) is mirrored.
  if (this->flip_[half]) {
    this->half_y0_ = this->height_ - this->y_high_;
    this->half_y1_ = this->height_ - this->y_low_;
  } else {
    this->half_y0_ = this->y_low_;
    this->half_y1_ = this->y_high_;
  }
  return true;
}

void EPaperSSD1608Dual::begin_half_window_(uint8_t half) {
  const uint8_t x_start = this->half_x0_;
  const uint8_t x_end = this->half_x1_ - 1;
  const uint16_t y_start = this->half_y0_;
  const uint16_t y_end = this->half_y1_ - 1;
  // Window and counters use the chip's local addresses: X is the source byte
  // (0..width/2/8-1), Y the gate line.
  this->write_command_half_(half, R44_RAM_X_START_END, {x_start, x_end});
  this->write_command_half_(half, R4E_RAM_X_COUNTER, {x_start});
  this->write_command_half_(half, R45_RAM_Y_START_END,
                            {(uint8_t) (y_start & 0xFF), (uint8_t) (y_start >> 8), (uint8_t) (y_end & 0xFF),
                             (uint8_t) (y_end >> 8)});
  this->write_command_half_(half, R4F_RAM_Y_COUNTER, {(uint8_t) (y_start & 0xFF), (uint8_t) (y_start >> 8)});
  this->write_command_half_(half, R24_WRITE_RAM, {});
}

bool HOT EPaperSSD1608Dual::transfer_data() {
  const auto start_time = millis();
  if (this->transfer_half_ < 0) {
    // Without partial update the display went to deep sleep (losing RAM), so
    // the whole buffer must be resent; otherwise round to byte boundaries.
    if (!this->is_using_partial_update_()) {
      this->x_low_ = 0;
      this->x_high_ = this->width_;
      this->y_low_ = 0;
      this->y_high_ = this->height_;
    }
    this->x_low_ &= ~7;
    this->x_high_ = (this->x_high_ + 7) & ~7;
    this->transfer_half_ = 0;
    ESP_LOGD(TAG, "Transfer x=%u..%u y=%u..%u", this->x_low_, this->x_high_, this->y_low_, this->y_high_);
  }

  while (this->transfer_half_ < 2) {
    const uint8_t half = this->transfer_half_;
    if (this->current_data_index_ == 0) {
      // Starting a new half: set its RAM window and enter RAM write mode.
      if (!this->setup_half_window_(half)) {
        this->transfer_half_++;
        continue;
      }
      this->begin_half_window_(half);
      this->current_data_index_ = this->half_y0_;
    }

    // A block of gate lines in one SPI transaction. The SSD1608 stays in RAM
    // write mode (command 0x24) across CS releases until another command is
    // sent, so a resumed block can simply continue writing data.
    const size_t row_length = this->half_x1_ - this->half_x0_;
    const size_t half_offset = (size_t) half * ((this->row_width_ + 1) / 2) + this->half_x0_;
    FixedVector<uint8_t> bytes_to_send{};
    bytes_to_send.init(row_length);
    ESP_LOGV(TAG, "Writing %u bytes at line %zu at %ums", row_length, this->current_data_index_,
             (unsigned) millis());
    GPIOPin *cs = half == 0 ? this->cs_pin_ : this->cs1_pin_;
    cs->digital_write(false);
    this->start_data_();
    while (this->current_data_index_ < this->half_y1_) {
      // RAM gate line -> buffer row; flipped halves read the buffer bottom-up.
      const size_t buffer_row = this->flip_[half] ? this->height_ - 1 - this->current_data_index_
                                                  : this->current_data_index_;
      const uint8_t *data = &this->buffer_[buffer_row * this->row_width_ + half_offset];
      for (size_t i = 0; i != row_length; i++) {
        bytes_to_send[i] = this->flip_[half] ? bit_reverse_(data[i]) : data[i];
      }
      this->write_array(&bytes_to_send.front(), row_length);  // NOLINT
      this->current_data_index_++;
      if (millis() - start_time > MAX_TRANSFER_TIME) {
        // Let the main loop run and come back next loop
        break;
      }
    }
    this->disable();
    cs->digital_write(true);
    if (this->current_data_index_ < this->half_y1_) {
      return false;  // block not finished; the window is not resent on resume
    }
    this->current_data_index_ = 0;
    this->transfer_half_++;
  }

  this->transfer_half_ = -1;  // ready for the next update
  return true;
}

void EPaperSSD1608Dual::refresh_screen(bool partial) {
  // With use_otp_lut the update sequence loads temperature + LUT from OTP
  // (0xFF also runs the initial-display stage). Without it, the LUT written by
  // the init sequence is used and temperature loading is skipped, matching
  // Adafruit's SSD1608 driver. A blank OTP otherwise leaves BUSY high forever.
  const uint8_t option = this->use_otp_lut_ ? (partial ? 0xF7 : 0xFF) : 0xC7;
  ESP_LOGD(TAG, "Refresh (%s), option 0x%02X, %s LUT", partial ? "partial" : "full", option,
           this->use_otp_lut_ ? "OTP" : "register");
  // 0xFF enables the full sequence including the initial display stage (the
  // anti-ghosting flash); 0xF7 skips it for a fast flicker-free refresh.
  this->write_command_both_(R22_DISPLAY_UPDATE_CONTROL_2, {option});
  // Activate the slave first: the busy line is the master's, so by the time it
  // reports idle the slave (sharing the master's clock) has finished too.
  const bool master_is_cs = !this->master_is_cs1_;
  this->write_command_(R20_MASTER_ACTIVATION, !master_is_cs, master_is_cs);
  this->write_command_(R20_MASTER_ACTIVATION, master_is_cs, !master_is_cs);
  delay(10);
}

void EPaperSSD1608Dual::deep_sleep() {
  // Deep sleep loses RAM so cannot be used with partial update
  if (!this->is_using_partial_update_()) {
    ESP_LOGD(TAG, "Deep sleep");
    this->write_command_both_(R10_DEEP_SLEEP, {0x01});
  }
}

uint8_t EPaperSSD1608Dual::bit_reverse_(uint8_t value) {
  value = (value & 0xF0) >> 4 | (value & 0x0F) << 4;
  value = (value & 0xCC) >> 2 | (value & 0x33) << 2;
  value = (value & 0xAA) >> 1 | (value & 0x55) << 1;
  return value;
}

void EPaperSSD1608Dual::dump_config() {
  epaper_spi::EPaperBase::dump_config();
  LOG_PIN("  CS Pin: ", this->cs_pin_);
  LOG_PIN("  CS1 Pin: ", this->cs1_pin_);
  ESP_LOGCONFIG(TAG, "  Master: %s, Flip: CS %s, CS1 %s, LUT: %s", this->master_is_cs1_ ? "CS1" : "CS",
                YESNO(this->flip_[0]), YESNO(this->flip_[1]), this->use_otp_lut_ ? "OTP" : "register");
}

}  // namespace esphome::epaper

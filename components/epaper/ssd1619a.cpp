#include "ssd1619a.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::epaper {

using epaper_spi::MAX_TRANSFER_TIME;

static constexpr const char *const TAG = "epaper.ssd1619a";

// SSD1619A register addresses (datasheet Rev 0.10, section 7 command table)
static constexpr uint8_t R10_DEEP_SLEEP = 0x10;
static constexpr uint8_t R12_SWRESET = 0x12;
static constexpr uint8_t R20_MASTER_ACTIVATION = 0x20;
static constexpr uint8_t R21_DISPLAY_UPDATE_CONTROL_1 = 0x21;
static constexpr uint8_t R22_DISPLAY_UPDATE_CONTROL_2 = 0x22;
static constexpr uint8_t R24_WRITE_RAM_BW = 0x24;
static constexpr uint8_t R26_WRITE_RAM_RED = 0x26;
static constexpr uint8_t R32_WRITE_LUT = 0x32;
static constexpr uint8_t R3A_DUMMY_LINE = 0x3A;
static constexpr uint8_t R3B_GATE_LINE_WIDTH = 0x3B;
static constexpr uint8_t R44_RAM_X_START_END = 0x44;
static constexpr uint8_t R45_RAM_Y_START_END = 0x45;
static constexpr uint8_t R4E_RAM_X_COUNTER = 0x4E;
static constexpr uint8_t R4F_RAM_Y_COUNTER = 0x4F;

// Full-update waveform (70 bytes, 0x32 layout). The refresh still asks the
// controller to load the OTP waveform (0x22 = 0xF7), but panels whose OTP is
// blank keep the LUT written here instead, so the update does not depend on a
// programmed OTP. Values are taken from the reference SSD1619A driver
// (createskyblue/ssd1619a-epd).
static constexpr uint8_t FULL_LUT[] = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// Partial-update waveform (70 bytes, 0x32 layout as in the SSD1619A
// datasheet). A partial refresh uses 0xC7, which does not load the waveform
// from OTP, so it must be written explicitly. Values are taken from the
// reference SSD1619A driver (createskyblue/ssd1619a-epd).
static constexpr uint8_t PARTIAL_LUT[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
    0x10, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static_assert(sizeof(FULL_LUT) == 70, "SSD1619A full LUT must be 70 bytes");
static_assert(sizeof(PARTIAL_LUT) == 70, "SSD1619A partial LUT must be 70 bytes");

bool EPaperSSD1619A::reset() {
  if (epaper_spi::EPaperBase::reset()) {
    // The base class releases RES# and returns immediately. Give the
    // controller time to leave reset, then restore the register defaults
    // with a software reset (it cannot exit deep sleep; the hardware reset
    // above is what does that).
    delay(10);
    this->command(R12_SWRESET);
    // BUSY goes high while the controller resets; a short wait avoids
    // sending the init sequence into a chip that is still resetting.
    delay(10);
    return true;
  }
  return false;
}

void EPaperSSD1619A::set_window_() {
  const auto x_start = (uint8_t) this->window_x0_;
  const auto x_end = (uint8_t) (this->window_x1_ - 1);
  const auto y_start = this->window_y0_;
  const auto y_end = (uint16_t) (this->window_y1_ - 1);
  // X is addressed in bytes (8 sources per address); Y is a 16-bit gate line.
  this->cmd_data(R44_RAM_X_START_END, {x_start, x_end});
  this->cmd_data(R45_RAM_Y_START_END,
                 {(uint8_t) (y_start & 0xFF), (uint8_t) (y_start >> 8), (uint8_t) (y_end & 0xFF),
                  (uint8_t) (y_end >> 8)});
  this->cmd_data(R4E_RAM_X_COUNTER, {x_start});
  this->cmd_data(R4F_RAM_Y_COUNTER, {(uint8_t) (y_start & 0xFF), (uint8_t) (y_start >> 8)});
}

bool EPaperSSD1619A::initialise(bool partial) {
  // Every update starts here, so drop any transfer state left over from an
  // update that did not finish.
  this->transfer_active_ = false;
  this->data_pass_ = 0;
  return epaper_spi::EPaperBase::initialise(partial);
}

bool HOT EPaperSSD1619A::transfer_data() {
  const auto start_time = millis();
  if (!this->transfer_active_) {
    // Without partial update the display went to deep sleep, so the whole
    // buffer must be resent; otherwise round to byte boundaries.
    if (!this->is_using_partial_update_()) {
      this->x_low_ = 0;
      this->x_high_ = this->width_;
      this->y_low_ = 0;
      this->y_high_ = this->height_;
    }
    this->x_low_ &= ~7;
    this->x_high_ = (this->x_high_ + 7) & ~7;
    this->window_x0_ = this->x_low_ / 8;
    this->window_x1_ = this->x_high_ / 8;
    this->window_y0_ = this->y_low_;
    this->window_y1_ = this->y_high_;
    this->data_pass_ = 0;
    this->transfer_line_ = this->window_y0_;
    this->transfer_active_ = true;
    ESP_LOGD(TAG, "Transfer x=%u..%u y=%u..%u", this->x_low_, this->x_high_, this->y_low_, this->y_high_);
  }

  // A full update writes the image to the BW RAM and white to the RED RAM so
  // the waveform has a clean baseline to compare against. A partial update
  // leaves the RED RAM untouched: it still holds the previous frame.
  const uint8_t passes = this->is_using_partial_update_() ? 1 : 2;
  const size_t row_length = this->window_x1_ - this->window_x0_;
  FixedVector<uint8_t> bytes_to_send{};
  bytes_to_send.init(row_length);

  while (this->data_pass_ < passes) {
    if (this->transfer_line_ == this->window_y0_) {
      // Starting a new plane: set the window and enter RAM write mode.
      this->set_window_();
      this->command(this->data_pass_ == 0 ? R24_WRITE_RAM_BW : R26_WRITE_RAM_RED);
    }

    // The controller stays in RAM write mode across CS releases until another
    // command is sent, so a resumed block can simply continue writing data.
    this->start_data_();
    while (this->transfer_line_ < this->window_y1_) {
      if (this->data_pass_ == 0) {
        const size_t offset = (size_t) this->transfer_line_ * this->row_width_ + this->window_x0_;
        for (size_t i = 0; i != row_length; i++)
          bytes_to_send[i] = this->buffer_[offset + i];
      } else {
        for (size_t i = 0; i != row_length; i++)
          bytes_to_send[i] = 0xFF;  // no red pixels
      }
      this->write_array(&bytes_to_send.front(), row_length);  // NOLINT
      this->transfer_line_++;
      if (millis() - start_time > MAX_TRANSFER_TIME) {
        // Let the main loop run and come back next loop
        this->disable();
        return false;
      }
    }
    this->disable();
    this->data_pass_++;
    this->transfer_line_ = this->window_y0_;
  }

  this->transfer_active_ = false;
  return true;
}

void EPaperSSD1619A::power_off() {
  // There is no power-off command: the update sequence (0x22) powers the
  // analog blocks down. Before the controller goes to deep sleep, copy the
  // displayed window into the RED RAM, which is the previous frame the next
  // differential (partial) update compares against.
  if (!this->is_using_partial_update_() || this->window_y1_ <= this->window_y0_)
    return;

  this->set_window_();
  this->command(R26_WRITE_RAM_RED);
  const size_t row_length = this->window_x1_ - this->window_x0_;
  FixedVector<uint8_t> bytes_to_send{};
  bytes_to_send.init(row_length);
  this->start_data_();
  for (uint16_t line = this->window_y0_; line < this->window_y1_; line++) {
    const size_t offset = (size_t) line * this->row_width_ + this->window_x0_;
    for (size_t i = 0; i != row_length; i++)
      bytes_to_send[i] = this->buffer_[offset + i];
    this->write_array(&bytes_to_send.front(), row_length);  // NOLINT
  }
  this->disable();
}

void EPaperSSD1619A::refresh_screen(bool partial) {
  // 0xF7 loads the waveform and temperature from OTP and runs the full
  // update sequence; 0xC7 uses the LUT currently in the registers.
  const uint8_t option = partial ? 0xC7 : 0xF7;
  ESP_LOGD(TAG, "Refresh (%s), option 0x%02X", partial ? "partial" : "full", option);
  // Normal (not inverted or bypassed) RAM content.
  this->cmd_data(R21_DISPLAY_UPDATE_CONTROL_1, {0x00});
  if (partial) {
    // 0xC7 does not load the waveform from OTP, so write the partial LUT and
    // the timing values tuned for it first.
    this->cmd_data(R32_WRITE_LUT, PARTIAL_LUT, sizeof(PARTIAL_LUT));
    this->cmd_data(R3A_DUMMY_LINE, {0x21});
    this->cmd_data(R3B_GATE_LINE_WIDTH, {0x06});
  } else {
    // 0xF7 loads the OTP waveform, but a blank OTP leaves this LUT in place.
    this->cmd_data(R32_WRITE_LUT, FULL_LUT, sizeof(FULL_LUT));
    this->cmd_data(R3A_DUMMY_LINE, {0x1A});
    this->cmd_data(R3B_GATE_LINE_WIDTH, {0x0B});
  }
  this->cmd_data(R22_DISPLAY_UPDATE_CONTROL_2, {option});
  this->command(R20_MASTER_ACTIVATION);
  delay(10);
}

void EPaperSSD1619A::deep_sleep() {
  // Mode 1 retains RAM, which the next partial update needs; mode 2 drops it,
  // which is fine when every update rewrites the whole buffer.
  const uint8_t mode = this->is_using_partial_update_() ? 0x01 : 0x03;
  ESP_LOGD(TAG, "Deep sleep (mode %u)", mode);
  this->cmd_data(R10_DEEP_SLEEP, {mode});
}

void EPaperSSD1619A::dump_config() {
  epaper_spi::EPaperBase::dump_config();
  ESP_LOGCONFIG(TAG, "  Driver: SSD1619A (single controller)");
}

}  // namespace esphome::epaper

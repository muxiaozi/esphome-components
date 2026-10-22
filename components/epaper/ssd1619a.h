#pragma once

#include "esphome/components/epaper_spi/epaper_spi.h"

namespace esphome::epaper {

/**
 * Monochrome 400x300 e-paper displays driven by a single SSD1619A controller
 * (e.g. HINK-E042A13-A0).
 *
 * The SSD1619A has 400 source and 300 gate outputs, so unlike the dual
 * SSD1608 panels it drives the whole panel by itself and uses the normal SPI
 * bus chip-select handling.
 *
 * The controller has two RAM planes: BW (0x24) and RED (0x26). On a
 * black/white panel the RED plane also holds the previously displayed image
 * that the waveform compares against during a differential (partial) update,
 * so the driver keeps it in sync with what is on screen.
 *
 * A full refresh writes a fallback waveform and uses 0x22 = 0xF7, which
 * loads the waveform and temperature from OTP on programmed controllers.
 * A partial refresh uses 0x22 = 0xC7, which never loads the OTP waveform, so
 * the partial LUT is written to 0x32 first.
 */
class EPaperSSD1619A : public epaper_spi::EPaperBase {
 public:
  EPaperSSD1619A(const char *name, uint16_t width, uint16_t height, const uint8_t *init_sequence,
                 size_t init_sequence_length)
      : epaper_spi::EPaperBase(name, width, height, init_sequence, init_sequence_length, display::DISPLAY_TYPE_BINARY) {
    this->buffer_length_ = (size_t) this->row_width_ * height;
  }

  void dump_config() override;

 protected:
  bool reset() override;
  bool initialise(bool partial) override;
  bool transfer_data() override;
  void refresh_screen(bool partial) override;
  void power_on() override {}
  void power_off() override;
  void deep_sleep() override;

  /// Send the RAM window and address counters for the current update.
  void set_window_();

  /// Source byte window of the current update (local RAM X addresses, half-open).
  uint16_t window_x0_{}, window_x1_{};
  /// Gate line window of the current update (half-open).
  uint16_t window_y0_{}, window_y1_{};

  /// Transfer progress: which RAM plane is being written (0 = BW, 1 = RED)
  /// and the next gate line to send.
  bool transfer_active_{false};
  uint8_t data_pass_{0};
  uint16_t transfer_line_{0};
};

}  // namespace esphome::epaper

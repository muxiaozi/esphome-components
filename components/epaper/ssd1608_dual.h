#pragma once

#include "esphome/components/epaper_spi/epaper_spi.h"

namespace esphome::epaper {

/**
 * Monochrome e-paper displays driven by two cascaded SSD1608 controllers
 * (e.g. HINK-E042A03-A1 4.2" 400x300).
 *
 * The SSD1608 is a 240x320 controller; such panels use two of them in the
 * cascade mode of the SSD1608 datasheet (section 6.11): the master chip
 * supplies the clock and the booster voltages, the slave chip's
 * oscillator/booster are disabled and its BUSY pin is left floating. Each
 * chip drives one half of the panel width and has its own chip select;
 * SCLK, MOSI, D/C and RESET are shared.
 *
 * RAM layout (datasheet section 6.2): X is the source direction, one byte
 * covering 8 sources (MSB = lowest source), Y is the gate direction. With
 * data entry mode 0x03 (X/Y increment) the data stream of one chip is
 * (width/2/8) bytes per gate line, which matches the rows of ESPHome's 1bpp
 * buffer sliced in half.
 *
 * The SSD1608 has no separate power on/off commands: the 0x22 display update
 * sequence option plus 0x20 master activation handle everything. 0xFF
 * includes the initial-display stage (full refresh with anti-ghosting
 * flashing), 0xF7 skips it (flicker-free partial refresh).
 */
class EPaperSSD1608Dual : public epaper_spi::EPaperBase {
 public:
  EPaperSSD1608Dual(const char *name, uint16_t width, uint16_t height, const uint8_t *init_sequence,
                    size_t init_sequence_length)
      : epaper_spi::EPaperBase(name, width, height, init_sequence, init_sequence_length, display::DISPLAY_TYPE_BINARY) {
    this->buffer_length_ = (size_t) this->row_width_ * height;
  }

  void set_cs_pins(GPIOPin *cs, GPIOPin *cs1) {
    this->cs_pin_ = cs;
    this->cs1_pin_ = cs1;
  }
  /// True when the master controller (the chip the busy_pin is wired to) is on cs1.
  void set_master(bool master_is_cs1) { this->master_is_cs1_ = master_is_cs1; }
  /// Per-chip 180 degree mounting compensation (chip rotated on the glass).
  void set_flip(bool flip_cs, bool flip_cs1) {
    this->flip_[0] = flip_cs;
    this->flip_[1] = flip_cs1;
  }
  /// false = write LUT/VCOM from the init sequence and refresh with 0x22=0xC7
  /// instead of loading the waveform from the controller's OTP.
  void set_use_otp_lut(bool use_otp_lut) { this->use_otp_lut_ = use_otp_lut; }

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  bool reset() override;
  bool initialise(bool partial) override;
  bool transfer_data() override;
  void refresh_screen(bool partial) override;
  void power_on() override {}
  void power_off() override {}
  void deep_sleep() override;

  /**
   * Send a command (and optional data) selecting one or both controllers.
   * Both chip-selects are active-low and managed directly by this driver.
   * @param use_cs  assert CS (the cs_pin chip) for this transaction
   * @param use_cs1 assert CS1 (the cs1_pin chip) for this transaction
   */
  void write_command_(uint8_t command, const uint8_t *data, size_t length, bool use_cs, bool use_cs1);
  void write_command_(uint8_t command, std::initializer_list<uint8_t> data, bool use_cs, bool use_cs1) {
    this->write_command_(command, data.begin(), data.size(), use_cs, use_cs1);
  }
  void write_command_(uint8_t command, bool use_cs, bool use_cs1) {
    this->write_command_(command, nullptr, 0, use_cs, use_cs1);
  }
  /// Send a command to a single half of the panel (0 = cs, 1 = cs1).
  void write_command_half_(uint8_t half, uint8_t command, std::initializer_list<uint8_t> data) {
    this->write_command_(command, data.begin(), data.size(), half == 0, half == 1);
  }
  /// Send a command to both controllers at once (shared SCLK/MOSI).
  void write_command_both_(uint8_t command, std::initializer_list<uint8_t> data = {}) {
    this->write_command_(command, data.begin(), data.size(), true, true);
  }

  /// Replay the init sequence to both controllers (the base class only has a single-CS path).
  void send_init_sequence_both_();
  /// Compute the RAM window of one half from the dirty rectangle. False if the half needs no update.
  bool setup_half_window_(uint8_t half);
  /// Send the window/address-counter commands and the RAM write command for one half.
  void begin_half_window_(uint8_t half);
  static uint8_t bit_reverse_(uint8_t value);

  GPIOPin *cs_pin_{};
  GPIOPin *cs1_pin_{};
  bool master_is_cs1_{};
  bool flip_[2]{};
  bool use_otp_lut_{true};

  /// Transfer progress: which half is being written (-1 = not started, 0 = cs, 1 = cs1).
  int8_t transfer_half_{-1};
  /// Byte window within the current half (local X addresses, half-open).
  uint16_t half_x0_{}, half_x1_{};
  /// Gate line window of the current half (local Y addresses, half-open).
  uint16_t half_y0_{}, half_y1_{};

  /// Diagnostics: last state seen by loop(), and when it was entered.
  epaper_spi::EPaperState last_logged_state_{epaper_spi::EPaperState::IDLE};
  uint32_t last_state_change_{};
};

}  // namespace esphome::epaper

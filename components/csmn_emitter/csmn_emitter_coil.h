#pragma once

#include "esphome/components/uart/uart.h"

#include "csmn_emitter.h"

namespace esphome {
namespace csmn_emitter {

class GPIOBinarySensorStore {
 public:
  void setup(InternalGPIOPin *pin, gpio::InterruptType type, Component *component);

  static void gpio_intr(GPIOBinarySensorStore *arg);

  bool get_state() const {
    // No lock needed: state_ is atomically updated by ISR
    // Volatile ensures we read the latest value
    return this->state_;
  }

  bool is_changed() const {
    // Simple read of volatile bool - no clearing here
    return this->changed_;
  }

  void clear_changed() {
    // Separate method to clear the flag
    this->changed_ = false;
  }

 protected:
  ISRInternalGPIOPin isr_pin_;
  volatile bool state_{false};
  volatile bool last_state_{false};
  volatile bool changed_{false};
  Component *component_{nullptr};  // Pointer to the component for enable_loop_soon_any_context()
};

class CsmnEmitterCoil : public CsmnEmitter, public uart::UARTDevice {
 public:
  // Impl Component
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Impl CsmnEmitter
  void change_trigger_mode(TriggerMode mode) override;
  void set_vibration_intensity(uint8_t intensity) override;
  void set_num_bullets(uint8_t num) override;

  void set_trigger_pin(GPIOPin *pin) { this->trigger_pin_ = pin; }
  void set_trigger_mode_pin(GPIOPin *pin) { this->trigger_mode_pin_ = pin; }

 protected:
  GPIOPin *trigger_pin_;
  GPIOPin *trigger_mode_pin_;
  GPIOBinarySensorStore trigger_pin_store_;
  GPIOBinarySensorStore trigger_mode_pin_store_;
};

} // namespace csmn_emitter
} // namespace esphome
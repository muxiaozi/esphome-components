#include "csmn_emitter_coil.h"

namespace esphome {
namespace csmn_emitter {

static const char *const TAG = "csmn_emitter.coil";

void GPIOBinarySensorStore::setup(InternalGPIOPin *pin, gpio::InterruptType type, Component *component) {
  pin->setup();
  this->isr_pin_ = pin->to_isr();
  this->component_ = component;

  // Read initial state
  this->last_state_ = pin->digital_read();
  this->state_ = this->last_state_;

  // Attach interrupt - from this point on, any changes will be caught by the interrupt
  pin->attach_interrupt(&GPIOBinarySensorStore::gpio_intr, this, type);
}

void IRAM_ATTR GPIOBinarySensorStore::gpio_intr(GPIOBinarySensorStore *arg) {
  bool new_state = arg->isr_pin_.digital_read();
  if (new_state != arg->last_state_) {
    arg->state_ = new_state;
    arg->last_state_ = new_state;
    arg->changed_ = true;
    // Wake up the component from its disabled loop state
    if (arg->component_ != nullptr) {
      arg->component_->enable_loop_soon_any_context();
    }
  }
}

void CsmnEmitterCoil::setup() {
  auto *internal_trigger_pin = static_cast<InternalGPIOPin *>(this->trigger_pin_);
  this->trigger_pin_store_.setup(internal_trigger_pin, gpio::INTERRUPT_ANY_EDGE, this);

  auto *internal_trigger_mode_pin = static_cast<InternalGPIOPin *>(this->trigger_mode_pin_);
  this->trigger_mode_pin_store_.setup(internal_trigger_mode_pin, gpio::INTERRUPT_ANY_EDGE, this);
  
  this->trigger_binary_sensor_->publish_initial_state(false);
  this->bolt_binary_sensor_->publish_initial_state(false);
  this->battery_level_sensor_->publish_state(100.0);
  this->emitter_type_text_sensor_->publish_state("Coil");
}

void CsmnEmitterCoil::loop() {
  if (this->trigger_pin_store_.is_changed()) {
    bool trigger_state = this->trigger_pin_store_.get_state();
    this->trigger_binary_sensor_->publish_state(trigger_state);
    this->trigger_pin_store_.clear_changed();
  }
  if (this->trigger_mode_pin_store_.is_changed()) {
    bool bolt_state = this->trigger_mode_pin_store_.get_state();
    // TODO: handle trigger mode
    this->trigger_mode_pin_store_.clear_changed();
  }
}

void CsmnEmitterCoil::dump_config() {
  ESP_LOGCONFIG(TAG, "CsmnEmitter:");
  ESP_LOGCONFIG(TAG, "  Type: Coil");
  ESP_LOGCONFIG(TAG, "  Trigger Pin: %s (%s)", this->trigger_pin_->dump_summary().c_str(),
                this->trigger_pin_->is_internal() ? "Internal" : "External");
  ESP_LOGCONFIG(TAG, "  Trigger Mode Pin: %s (%s)", this->trigger_mode_pin_->dump_summary().c_str(),
                this->trigger_mode_pin_->is_internal() ? "Internal" : "External");
}

void CsmnEmitterCoil::change_trigger_mode(TriggerMode mode) {
  ESP_LOGI(TAG, "Changing trigger mode to %d", mode);
}

void CsmnEmitterCoil::set_vibration_intensity(uint8_t intensity) {
  ESP_LOGI(TAG, "Setting vibration intensity to %d", intensity);
}

void CsmnEmitterCoil::set_num_bullets(uint8_t num) {
  ESP_LOGI(TAG, "Setting number of bullets to %d", num);
}

}  // namespace csmn_emitter
}  // namespace esphome
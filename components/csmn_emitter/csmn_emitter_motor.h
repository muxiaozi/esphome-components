#pragma once

#include "csmn_emitter.h"

namespace esphome {
namespace csmn_emitter {

class CsmnEmitterMotor : public CsmnEmitter {
 public:
  // Impl Component
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Impl CsmnEmitter
  void change_trigger_mode(TriggerMode mode) override;
  void set_vibration_intensity(uint8_t intensity) override;
  void set_num_bullets(uint8_t num) override;
};

} // namespace csmn_emitter
} // namespace esphome
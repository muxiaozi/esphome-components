#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"

#include <cinttypes>
#include <vector>
#include <queue>
#include <memory>
#include <list>

namespace esphome {
namespace csmn_emitter {

enum TriggerMode {
  TriggerModeSingleShot = 0,
  TriggerModeFullAuto = 1,
};

class CsmnEmitter : public Component {
#ifdef USE_SELECT
  SUB_SELECT(trigger_mode)
#endif
#ifdef USE_SENSOR
  SUB_SENSOR(battery_level)
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(emitter_type)
#endif
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(trigger)
  SUB_BINARY_SENSOR(bolt)
#endif
#ifdef USE_NUMBER
  SUB_NUMBER(vibration_intensity)
  SUB_NUMBER(num_bullets)
#endif

 public:
  virtual void change_trigger_mode(TriggerMode mode) = 0;
  virtual void set_vibration_intensity(uint8_t intensity) = 0;
  virtual void set_num_bullets(uint8_t num) = 0;
};

class TriggerModeSelect : public select::Select, public Parented<CsmnEmitter> {
 protected:
  void control(const std::string &value) override {
    this->publish_state(value);
    if (value == "SingleShot") {
      this->get_parent()->change_trigger_mode(TriggerModeSingleShot);
    } else if (value == "FullAuto") {
      this->get_parent()->change_trigger_mode(TriggerModeFullAuto);
    }
  }
};

class VibrationIntensityNumber : public number::Number, public Parented<CsmnEmitter> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->get_parent()->set_vibration_intensity(static_cast<uint8_t>(value));
  }
};

class NumBulletsNumber : public number::Number, public Parented<CsmnEmitter> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->get_parent()->set_num_bullets(static_cast<uint8_t>(value));
  }
};

}  // namespace csmn_emitter
}  // namespace esphome

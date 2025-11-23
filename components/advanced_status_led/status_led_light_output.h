#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace advanced_status_led {

enum StatusLedState {
  STATUS_LED_STATE_OK,
  STATUS_LED_STATE_ERROR,
  STATUS_LED_STATE_WARNING,
  STATUS_LED_STATE_NO_NETWORK,
  STATUS_LED_STATE_NO_API_CONNECTION,
  STATUS_LED_STATE_NO_MQTT_CONNECTION,
};

class StatusLEDLightOutput : public light::LightOutput, public Component {
 public:
  void set_output(output::FloatOutput *output) { output_ = output; }

  // Impl Component
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  float get_loop_priority() const override { return 50.0f; }

  // Impl light::LightOutput
  light::LightTraits get_traits() override;
  void setup_state(light::LightState *state) override;
  void write_state(light::LightState *state) override;

 protected:
  output::FloatOutput *output_{nullptr};
  light::LightState *lightstate_{};
  uint8_t last_app_state_{0xFF};
  StatusLedState last_status_led_state_{STATUS_LED_STATE_OK};
  void output_state_(float brightness);
};

}  // namespace advanced_status_led
}  // namespace esphome

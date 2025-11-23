#include "status_led_light_output.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/util.h"
#ifdef USE_ETHERNET
#include "esphome/components/ethernet/ethernet_component.h"
#endif // USE_ETHERNET
#ifdef USE_WIFI
  #include "esphome/components/wifi/wifi_component.h"
#endif // USE_WIFI

#include <cinttypes>

namespace esphome {
namespace advanced_status_led {

static const char *const TAG = "advanced_status_led.light";

static StatusLedState status_led_state(uint8_t app_state) {
  if (app_state & STATUS_LED_ERROR) {
    return STATUS_LED_STATE_ERROR;
  } else if (app_state & STATUS_LED_WARNING) {
#ifdef USE_WIFI
    if (wifi::global_wifi_component && !wifi::global_wifi_component->is_connected()) {
      return STATUS_LED_STATE_NO_NETWORK;
    }
#endif // USE_WIFI
#ifdef USE_ETHERNET
    if (ethernet::global_eth_component && !ethernet::global_eth_component->is_connected()) {
      return STATUS_LED_STATE_NO_NETWORK;
    }
#endif // USE_ETHERNET
    if (!api_is_connected()) {
      return STATUS_LED_STATE_NO_API_CONNECTION;
    }
    if (!mqtt_is_connected()) {
      return STATUS_LED_STATE_NO_MQTT_CONNECTION;
    }
    return STATUS_LED_STATE_WARNING;
  } else {
    return STATUS_LED_STATE_OK;
  }
}

void StatusLEDLightOutput::loop() {
  uint8_t new_app_state = App.get_app_state() & STATUS_LED_MASK;

  // If it is a warning, there are multiple possibilities and further judgment is needed
  if (new_app_state == this->last_app_state_ && (new_app_state & STATUS_LED_WARNING) == 0) {
    return;
  }

  StatusLedState new_status_led_state = status_led_state(new_app_state);
  this->last_app_state_ = new_app_state;
  if (new_status_led_state == this->last_status_led_state_) {
    return;
  }

  ESP_LOGD(TAG, "New status led state %" PRIu8, new_status_led_state);

  switch (new_status_led_state) {
    case STATUS_LED_STATE_OK: {
      this->lightstate_->turn_on().set_effect("none").perform();
      break;
    }
    case STATUS_LED_STATE_NO_NETWORK: {
      this->lightstate_->turn_on().set_effect("No Network").perform();
      break;
    }
    case STATUS_LED_STATE_NO_API_CONNECTION: {
      this->lightstate_->turn_on().set_effect("No Api Connection").perform();
      break;
    }
    case STATUS_LED_STATE_NO_MQTT_CONNECTION: {
      this->lightstate_->turn_on().set_effect("No Mqtt Connection").perform();
      break;
    }
    case STATUS_LED_STATE_WARNING: {
      this->lightstate_->turn_on().set_effect("Warning").perform();
      break;
    }
    case STATUS_LED_STATE_ERROR: {
      this->lightstate_->turn_on().set_effect("Error").perform();
      break;
    }
    default: 
      break;
  }

  this->last_status_led_state_ = new_status_led_state;
}

void StatusLEDLightOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "Advanced Status LED:");
}

light::LightTraits StatusLEDLightOutput::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
  return traits;
}

void StatusLEDLightOutput::setup_state(light::LightState *state) {
  lightstate_ = state;
  ESP_LOGD(TAG, "'%s': Setting initial state", state->get_name().c_str());
  this->write_state(state);
}

void StatusLEDLightOutput::write_state(light::LightState *state) {
  float brightness;
  state->current_values_as_brightness(&brightness);
  this->output_state_(brightness);
}

void StatusLEDLightOutput::output_state_(float brightness) {
  if (this->output_ != nullptr)
    this->output_->set_level(brightness);
}

}  // namespace advanced_status_led
}  // namespace esphome

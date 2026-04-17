#include "event_hub.h"

#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/util.h"
#ifdef USE_ETHERNET
#include "esphome/components/ethernet/ethernet_component.h"
#endif  // USE_ETHERNET
#ifdef USE_WIFI
#include "esphome/components/wifi/wifi_component.h"
#endif  // USE_WIFI

namespace esphome::event_hub {

static const char *const TAG = "event_hub";

void EventHub::update() {
  uint8_t status_led_state = App.get_app_state() & STATUS_LED_MASK;
  if (status_led_state & STATUS_LED_ERROR) {
    if (!this->last_error_) {
      this->error_callback_.call();
      this->last_error_ = true;
    }
  } else {
    this->last_error_ = false;
  }

#ifdef USE_WIFI
  if (!wifi::global_wifi_component->is_connected()) {
    if (this->last_wifi_connected_) {
      this->network_disconnected_callback_.call();
      this->wifi_disconnected_callback_.call();
      this->last_wifi_connected_ = false;
    }
  } else {
    if (!this->last_wifi_connected_) {
      this->network_connected_callback_.call();
      this->wifi_connected_callback_.call();
      this->last_wifi_connected_ = true;
    }
  }
#endif  // USE_WIFI

#ifdef USE_ETHERNET
  if (!ethernet::global_eth_component->is_connected()) {
    if (this->last_eth_connected_) {
      this->network_disconnected_callback_.call();
      this->eth_disconnected_callback_.call();
      this->last_eth_connected_ = false;
    }
  } else {
    if (!this->last_eth_connected_) {
      this->network_connected_callback_.call();
      this->eth_connected_callback_.call();
      this->last_eth_connected_ = true;
    }
  }
#endif  // USE_ETHERNET

  if (!this->last_wifi_connected_ && !this->last_eth_connected_) {
    return;
  }

  if (!api_is_connected()) {
    if (this->last_api_connected_) {
      this->api_disconnected_callback_.call();
      this->last_api_connected_ = false;
    }
  } else {
    if (!this->last_api_connected_) {
      this->api_connected_callback_.call();
      this->last_api_connected_ = true;
    }
  }

  if (!mqtt_is_connected()) {
    if (this->last_mqtt_connected_) {
      this->mqtt_disconnected_callback_.call();
      this->last_mqtt_connected_ = false;
    }
  } else {
    if (!this->last_mqtt_connected_) {
      this->mqtt_connected_callback_.call();
      this->last_mqtt_connected_ = true;
    }
  }

  if (status_led_state & STATUS_LED_WARNING) {
    if (!this->last_warning_) {
      this->warning_callback_.call();
      this->last_warning_ = true;
    }
  } else {
    this->last_warning_ = false;
  }

  if (status_led_state == STATUS_LED_OK) {
    if (!this->last_ok_) {
      this->ok_callback_.call();
      this->last_ok_ = true;
    }
  } else {
    this->last_ok_ = false;
  }
}

}  // namespace esphome::event_hub
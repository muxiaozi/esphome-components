#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome::event_hub {

class EventHub : public PollingComponent {
 public:
  template<typename F> void add_on_ok_callback(F &&callback) { this->ok_callback_.add(std::forward<F>(callback)); }

  template<typename F> void add_on_error_callback(F &&callback) {
    this->error_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_warning_callback(F &&callback) {
    this->warning_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_network_connected_callback(F &&callback) {
    this->network_connected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_network_disconnected_callback(F &&callback) {
    this->network_disconnected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_wifi_connected_callback(F &&callback) {
    this->wifi_connected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_wifi_disconnected_callback(F &&callback) {
    this->wifi_disconnected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_eth_connected_callback(F &&callback) {
    this->eth_connected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_eth_disconnected_callback(F &&callback) {
    this->eth_disconnected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_api_connected_callback(F &&callback) {
    this->api_connected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_api_disconnected_callback(F &&callback) {
    this->api_disconnected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_mqtt_connected_callback(F &&callback) {
    this->mqtt_connected_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_mqtt_disconnected_callback(F &&callback) {
    this->mqtt_disconnected_callback_.add(std::forward<F>(callback));
  }

  virtual void update();

 protected:
  LazyCallbackManager<void()> ok_callback_{};
  LazyCallbackManager<void()> error_callback_{};
  LazyCallbackManager<void()> warning_callback_{};
  LazyCallbackManager<void()> network_connected_callback_{};
  LazyCallbackManager<void()> network_disconnected_callback_{};
  LazyCallbackManager<void()> wifi_connected_callback_{};
  LazyCallbackManager<void()> wifi_disconnected_callback_{};
  LazyCallbackManager<void()> eth_connected_callback_{};
  LazyCallbackManager<void()> eth_disconnected_callback_{};
  LazyCallbackManager<void()> api_connected_callback_{};
  LazyCallbackManager<void()> api_disconnected_callback_{};
  LazyCallbackManager<void()> mqtt_connected_callback_{};
  LazyCallbackManager<void()> mqtt_disconnected_callback_{};

  bool last_ok_{false};
  bool last_error_{false};
  bool last_warning_{false};
  bool last_wifi_connected_{false};
  bool last_eth_connected_{false};
  bool last_api_connected_{false};
  bool last_mqtt_connected_{false};
};

}  // namespace esphome::event_hub
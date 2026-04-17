#pragma once

#include "event_hub.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome::event_hub {

class EventHubOkTrigger : public Trigger<> {
 public:
  EventHubOkTrigger(EventHub *event_hub) {
    event_hub->add_on_ok_callback([this]() { this->trigger(); });
  }
};

class EventHubErrorTrigger : public Trigger<> {
 public:
  EventHubErrorTrigger(EventHub *event_hub) {
    event_hub->add_on_error_callback([this]() { this->trigger(); });
  }
};

class EventHubWarningTrigger : public Trigger<> {
 public:
  EventHubWarningTrigger(EventHub *event_hub) {
    event_hub->add_on_warning_callback([this]() { this->trigger(); });
  }
};

class EventHubNetworkConnectedTrigger : public Trigger<> {
 public:
  EventHubNetworkConnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_network_connected_callback([this]() { this->trigger(); });
  }
};

class EventHubNetworkDisconnectedTrigger : public Trigger<> {
 public:
  EventHubNetworkDisconnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_network_disconnected_callback([this]() { this->trigger(); });
  }
};

class EventHubWifiConnectedTrigger : public Trigger<> {
 public:
  EventHubWifiConnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_wifi_connected_callback([this]() { this->trigger(); });
  }
};

class EventHubWifiDisconnectedTrigger : public Trigger<> {
 public:
  EventHubWifiDisconnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_wifi_disconnected_callback([this]() { this->trigger(); });
  }
};

class EventHubEthConnectedTrigger : public Trigger<> {
 public:
  EventHubEthConnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_eth_connected_callback([this]() { this->trigger(); });
  }
};

class EventHubEthDisconnectedTrigger : public Trigger<> {
 public:
  EventHubEthDisconnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_eth_disconnected_callback([this]() { this->trigger(); });
  }
};

class EventHubApiConnectedTrigger : public Trigger<> {
 public:
  EventHubApiConnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_api_connected_callback([this]() { this->trigger(); });
  }
};

class EventHubApiDisconnectedTrigger : public Trigger<> {
 public:
  EventHubApiDisconnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_api_disconnected_callback([this]() { this->trigger(); });
  }
};

class EventHubMqttConnectedTrigger : public Trigger<> {
 public:
  EventHubMqttConnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_mqtt_connected_callback([this]() { this->trigger(); });
  }
};

class EventHubMqttDisconnectedTrigger : public Trigger<> {
 public:
  EventHubMqttDisconnectedTrigger(EventHub *event_hub) {
    event_hub->add_on_mqtt_disconnected_callback([this]() { this->trigger(); });
  }
};

}  // namespace esphome::event_hub

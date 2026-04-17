# Event Hub

Example:

```yaml

event_hub:
  id: event_hub
  on_error:
    - logger.log: "Error event received"
  on_warning:
    - logger.log: "Warning event received"
  on_wifi_connected:
    - logger.log: "WiFi connected event received"
  on_wifi_disconnected:
    - logger.log: "WiFi disconnected event received"
  on_eth_connected:
    - logger.log: "Ethernet connected event received"
  on_eth_disconnected:
    - logger.log: "Ethernet disconnected event received"
  on_network_connected:
    - logger.log: "Network connected event received"
  on_network_disconnected:
    - logger.log: "Network disconnected event received"
  on_api_connected:
    - logger.log: "API connected event received"
  on_api_disconnected:
    - logger.log: "API disconnected event received"
  on_mqtt_connected:
    - logger.log: "MQTT connected event received"
  on_mqtt_disconnected:
    - logger.log: "MQTT disconnected event received"
```

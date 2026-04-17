import esphome.codegen as cg
from esphome import automation
import esphome.config_validation as cv
from esphome.const import CONF_OUTPUT, CONF_ID

CODEOWNERS = ["@muxiaozi"]

CONF_ON_OK = "on_ok"
CONF_ON_ERROR = "on_error"
CONF_ON_WARNING = "on_warning"
CONF_ON_NETWORK_CONNECTED = "on_network_connected"
CONF_ON_NETWORK_DISCONNECTED = "on_network_disconnected"
CONF_ON_WIFI_CONNECTED = "on_wifi_connected"
CONF_ON_WIFI_DISCONNECTED = "on_wifi_disconnected"
CONF_ON_ETH_CONNECTED = "on_eth_connected"
CONF_ON_ETH_DISCONNECTED = "on_eth_disconnected"
CONF_ON_API_CONNECTED = "on_api_connected"
CONF_ON_API_DISCONNECTED = "on_api_disconnected"
CONF_ON_MQTT_CONNECTED = "on_mqtt_connected"
CONF_ON_MQTT_DISCONNECTED = "on_mqtt_disconnected"

event_hub_ns = cg.esphome_ns.namespace("event_hub")
EventHub = event_hub_ns.class_(
    "EventHub", cg.PollingComponent
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(EventHub),
        cv.Optional(CONF_ON_OK): automation.validate_automation({}),
        cv.Optional(CONF_ON_ERROR): automation.validate_automation({}),
        cv.Optional(CONF_ON_WARNING): automation.validate_automation({}),
        cv.Optional(CONF_ON_NETWORK_CONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_NETWORK_DISCONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_WIFI_CONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_WIFI_DISCONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_ETH_CONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_ETH_DISCONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_API_CONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_API_DISCONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_MQTT_CONNECTED): automation.validate_automation({}),
        cv.Optional(CONF_ON_MQTT_DISCONNECTED): automation.validate_automation({}),
    }
).extend(cv.polling_component_schema("100ms"))

_CALLBACK_AUTOMATIONS = (
    automation.CallbackAutomation(CONF_ON_OK, "add_on_ok_callback"),
    automation.CallbackAutomation(CONF_ON_ERROR, "add_on_error_callback"),
    automation.CallbackAutomation(CONF_ON_WARNING, "add_on_warning_callback"),
    automation.CallbackAutomation(CONF_ON_NETWORK_CONNECTED, "add_on_network_connected_callback"),
    automation.CallbackAutomation(CONF_ON_NETWORK_DISCONNECTED, "add_on_network_disconnected_callback"),
    automation.CallbackAutomation(CONF_ON_WIFI_CONNECTED, "add_on_wifi_connected_callback"),
    automation.CallbackAutomation(CONF_ON_WIFI_DISCONNECTED, "add_on_wifi_disconnected_callback"),
    automation.CallbackAutomation(CONF_ON_ETH_CONNECTED, "add_on_eth_connected_callback"),
    automation.CallbackAutomation(CONF_ON_ETH_DISCONNECTED, "add_on_eth_disconnected_callback"),
    automation.CallbackAutomation(CONF_ON_API_CONNECTED, "add_on_api_connected_callback"),
    automation.CallbackAutomation(CONF_ON_API_DISCONNECTED, "add_on_api_disconnected_callback"),
    automation.CallbackAutomation(CONF_ON_MQTT_CONNECTED, "add_on_mqtt_connected_callback"),
    automation.CallbackAutomation(CONF_ON_MQTT_DISCONNECTED, "add_on_mqtt_disconnected_callback"),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await automation.build_callback_automations(var, config, _CALLBACK_AUTOMATIONS)

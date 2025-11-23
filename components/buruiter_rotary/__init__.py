import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import modbus, sensor, select, button
from esphome.const import (
    CONF_ID,
    CONF_RESOLUTION,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_NONE,
    DEVICE_CLASS_WIND_DIRECTION,
    DEVICE_CLASS_SPEED,
    STATE_CLASS_MEASUREMENT,
    UNIT_DEGREES,
    UNIT_REVOLUTIONS_PER_MINUTE
)

CONF_MODBUS_ID = "modbus_id"
CONF_SINGLE_CYCLE = "single_cycle"
CONF_ANGULAR_VELOCITY = "angular_velocity"
CONF_DIRECTION = "direction"
CONF_RESET_ZERO = "reset_zero"
CONF_RESET_CENTER = "reset_center"

CODEOWNERS = ["hongwei.hu@foxmail.com"]
DEPENDENCIES = ["modbus", "sensor", "button", "select"]
AUTO_LOAD = ["sensor", "button", "select"]

buruiter_rotary_ns = cg.esphome_ns.namespace("buruiter_rotary")
BuruiterRotary = buruiter_rotary_ns.class_("BuruiterRotary", modbus.ModbusDevice, cg.PollingComponent)
DirectionSelect = buruiter_rotary_ns.class_("DirectionSelect", select.Select)
ResetZeroButton = buruiter_rotary_ns.class_("ResetZeroButton", button.Button)
ResetCenterButton = buruiter_rotary_ns.class_("ResetCenterButton", button.Button)

# 单圈精度
SINGLE_CYCLE_RESOLUTION = {
    10: 1024,
    12: 4096,
    15: 32768,
    17: 131072,
}

# 旋转方向
DIRECTION_OPTIONS = [
    "CW",
    "CCW"
]

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.declare_id(BuruiterRotary),
    cv.Required(CONF_MODBUS_ID): cv.declare_id(modbus.Modbus),
    cv.Optional(CONF_RESOLUTION, default=10): cv.one_of(*SINGLE_CYCLE_RESOLUTION, int=True),
    cv.Optional(CONF_SINGLE_CYCLE): sensor.sensor_schema(
        unit_of_measurement=UNIT_DEGREES,
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT,
        device_class=DEVICE_CLASS_WIND_DIRECTION,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_ANGULAR_VELOCITY): sensor.sensor_schema(
        unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT,
        device_class=DEVICE_CLASS_SPEED,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_DIRECTION): select.select_schema(
        DirectionSelect,
        entity_category=ENTITY_CATEGORY_CONFIG
    ),
    cv.Optional(CONF_RESET_ZERO): button.button_schema(
        ResetZeroButton,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_RESET_CENTER): button.button_schema(
        ResetCenterButton,
        entity_category=ENTITY_CATEGORY_NONE
    ),
}).extend(modbus.modbus_device_schema(1)).extend(cv.polling_component_schema("100ms"))


async def to_code(config):
    parent = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(parent, config)

    await modbus.register_modbus_device(parent, config)

    if resolution := SINGLE_CYCLE_RESOLUTION.get(config[CONF_RESOLUTION]):
        cg.add(parent.set_resolution(resolution))

    if single_cycle_config := config.get(CONF_SINGLE_CYCLE):
        s = await sensor.new_sensor(single_cycle_config)
        cg.add(parent.set_single_cycle_sensor(s))

    if angular_velocity_config := config.get(CONF_ANGULAR_VELOCITY):
        s = await sensor.new_sensor(angular_velocity_config)
        cg.add(parent.set_angular_velocity_sensor(s))

    if direction_config := config.get(CONF_DIRECTION):
        s = await select.new_select(
            direction_config,
            options=DIRECTION_OPTIONS
        )
        await cg.register_parented(s, parent)
        cg.add(parent.set_direction_select(s))

    if reset_zero_config := config.get(CONF_RESET_ZERO):
        s = await button.new_button(reset_zero_config)
        await cg.register_parented(s, parent)
        cg.add(parent.set_reset_zero_button(s))

    if reset_center_config := config.get(CONF_RESET_CENTER):
        s = await button.new_button(reset_center_config)
        await cg.register_parented(s, parent)
        cg.add(parent.set_reset_center_button(s))



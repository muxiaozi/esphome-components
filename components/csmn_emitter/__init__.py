import esphome.codegen as cg
from esphome import pins
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, text_sensor, number, select, uart
from esphome.const import (
    CONF_ID,
    CONF_BATTERY_LEVEL,
    CONF_TYPE,
    UNIT_PERCENT,
    ENTITY_CATEGORY_NONE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_NONE,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_BATTERY,
    UNIT_EMPTY,
    DEVICE_CLASS_EMPTY,
)

CONF_VIBRATION_INTENSITY = "vibration_intensity"
CONF_NUM_BULLETS = "num_bullets"
CONF_EMITTER_TYPE = "emitter_type"
CONF_TRIGGER_MODE = "trigger_mode"
CONF_TRIGGER = "trigger"
CONF_BOLT = "bolt"
CONF_TRIGGER_PIN = "trigger_pin"
CONF_TRIGGER_MODE_PIN = "trigger_mode_pin"

CODEOWNERS = ["hongwei.hu@foxmail.com"]
DEPENDENCIES = ["sensor", "binary_sensor", "text_sensor", "number", "select"]
AUTO_LOAD = ["sensor", "binary_sensor", "text_sensor", "number", "select"]

csmn_emitter_ns = cg.esphome_ns.namespace("csmn_emitter")
CsmnEmitter = csmn_emitter_ns.class_("CsmnEmitter", cg.Component)
CsmnEmitterCoil = csmn_emitter_ns.class_("CsmnEmitterCoil", CsmnEmitter, uart.UARTDevice)
CsmnEmitterMotor = csmn_emitter_ns.class_("CsmnEmitterMotor", CsmnEmitter)
VibrationIntensityNumber = csmn_emitter_ns.class_("VibrationIntensityNumber", number.Number)
NumBulletsNumber = csmn_emitter_ns.class_("NumBulletsNumber", number.Number)
TriggerModeSelect = csmn_emitter_ns.class_("TriggerModeSelect", select.Select)

EMITTER_SCHEMA = cv.Schema({
    cv.Optional(CONF_VIBRATION_INTENSITY): number.number_schema(
        class_=VibrationIntensityNumber,
        unit_of_measurement=UNIT_PERCENT,
        device_class=DEVICE_CLASS_EMPTY,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_NUM_BULLETS): number.number_schema(
        class_=NumBulletsNumber,
        unit_of_measurement=UNIT_EMPTY,
        device_class=DEVICE_CLASS_EMPTY,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_EMITTER_TYPE): text_sensor.text_sensor_schema(
        device_class=DEVICE_CLASS_EMPTY,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_TRIGGER_MODE): select.select_schema(
        class_=TriggerModeSelect,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_TRIGGER): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_EMPTY,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_BOLT): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_EMPTY,
        entity_category=ENTITY_CATEGORY_NONE
    ),
    cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
        entity_category=ENTITY_CATEGORY_NONE,
        state_class=STATE_CLASS_MEASUREMENT
    ),
}).extend(cv.COMPONENT_SCHEMA)

TYPE_COIL = "coil"
TYPE_MOTOR = "motor"

TRIGGER_MODE_OPTIONS = [
    "SingleShot",
    "FullAuto",
]

CONFIG_SCHEMA = cv.typed_schema({
    TYPE_COIL: EMITTER_SCHEMA
        .extend(uart.UART_DEVICE_SCHEMA)
        .extend({
            cv.GenerateID(CONF_ID): cv.declare_id(CsmnEmitterCoil),
            cv.Required(CONF_TRIGGER_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_TRIGGER_MODE_PIN): pins.gpio_input_pin_schema,
        }),
    TYPE_MOTOR: EMITTER_SCHEMA
        .extend({
            cv.GenerateID(CONF_ID): cv.declare_id(CsmnEmitterMotor),
        }),
}, lower=True)

async def to_code(config):
    parent = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(parent, config)

    if config[CONF_TYPE] == TYPE_COIL:
        await uart.register_uart_device(parent, config)
        trigger_pin = await cg.gpio_pin_expression(config[CONF_TRIGGER_PIN])
        cg.add(parent.set_trigger_pin(trigger_pin))
        trigger_mode_pin = await cg.gpio_pin_expression(config[CONF_TRIGGER_MODE_PIN])
        cg.add(parent.set_trigger_mode_pin(trigger_mode_pin))

    if vibration_intensity_config := config.get(CONF_VIBRATION_INTENSITY):
        s = await number.new_number(vibration_intensity_config, min_value=0, max_value=100, step=1)
        cg.add(parent.set_vibration_intensity_number(s))
        await cg.register_parented(s, parent)

    if num_bullets_config := config.get(CONF_NUM_BULLETS):
        s = await number.new_number(num_bullets_config, min_value=0, max_value=255, step=1)
        cg.add(parent.set_num_bullets_number(s))
        await cg.register_parented(s, parent)

    if emitter_type_config := config.get(CONF_EMITTER_TYPE):
        s = await text_sensor.new_text_sensor(emitter_type_config)
        cg.add(parent.set_emitter_type_text_sensor(s))

    if trigger_mode_config := config.get(CONF_TRIGGER_MODE):
        s = await select.new_select(trigger_mode_config, options=TRIGGER_MODE_OPTIONS)
        cg.add(parent.set_trigger_mode_select(s))
        await cg.register_parented(s, parent)

    if trigger_config := config.get(CONF_TRIGGER):
        s = await binary_sensor.new_binary_sensor(trigger_config)
        cg.add(parent.set_trigger_binary_sensor(s))

    if bolt_config := config.get(CONF_BOLT):
        s = await binary_sensor.new_binary_sensor(bolt_config)
        cg.add(parent.set_bolt_binary_sensor(s))

    if battery_level_config := config.get(CONF_BATTERY_LEVEL):
        s = await sensor.new_sensor(battery_level_config)
        cg.add(parent.set_battery_level_sensor(s))
    

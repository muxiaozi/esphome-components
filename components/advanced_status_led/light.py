import esphome.codegen as cg
from esphome.components import light, output
import esphome.config_validation as cv
from esphome.const import CONF_OUTPUT, CONF_OUTPUT_ID

DEPENDENCIES = ["output"]

csmn_status_led_ns = cg.esphome_ns.namespace("advanced_status_led")
StatusLEDLightOutput = csmn_status_led_ns.class_(
    "StatusLEDLightOutput", light.LightOutput, cg.Component
)

CONFIG_SCHEMA = light.BRIGHTNESS_ONLY_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(StatusLEDLightOutput),
        cv.Required(CONF_OUTPUT): cv.use_id(output.FloatOutput),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    out = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(out))
    await cg.register_component(var, config)
    await light.register_light(var, config)

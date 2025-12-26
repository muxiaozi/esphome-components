import esphome.codegen as cg
from esphome import automation
from esphome.automation import maybe_simple_id
import esphome.config_validation as cv
from esphome import pins
from esphome.components import http_request, microphone
from esphome.components.http_request import CONF_HTTP_REQUEST_ID
from esphome.const import (
    CONF_ID,
    CONF_MISO_PIN,
    CONF_MOSI_PIN,
    CONF_CS_PIN,
    CONF_CLK_PIN,
    CONF_MICROPHONE,
    CONF_UUID
)

CONF_BASE_URL = "base_url"

CODEOWNERS = ["hongwei.hu@foxmail.com"]
DEPENDENCIES = ["http_request"]

csmn_keypoints_ns = cg.esphome_ns.namespace("csmn_keypoints")
CsmnKeyPoints = csmn_keypoints_ns.class_("CsmnKeyPoints", cg.Component)
StartAction = csmn_keypoints_ns.class_(
    "StartAction", automation.Action, cg.Parented.template(CsmnKeyPoints)
)
StopAction = csmn_keypoints_ns.class_(
    "StopAction", automation.Action, cg.Parented.template(CsmnKeyPoints)
)
ToggleAction = csmn_keypoints_ns.class_(
    "ToggleAction", automation.Action, cg.Parented.template(CsmnKeyPoints)
)

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend({
    cv.GenerateID(CONF_ID): cv.declare_id(CsmnKeyPoints),
    cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(http_request.HttpRequestComponent),
    cv.GenerateID(CONF_MICROPHONE): cv.use_id(microphone.Microphone),
    cv.Required(CONF_MISO_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_MOSI_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_CLK_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_BASE_URL): cv.url,
    cv.Required(CONF_UUID): cv.uuid,
})

async def to_code(config):
    http_request_component = await cg.get_variable(config[CONF_HTTP_REQUEST_ID])
    microphone_component = await cg.get_variable(config[CONF_MICROPHONE])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_http_request(http_request_component)) 
    cg.add(var.set_microphone(microphone_component))
    cg.add(var.set_miso_pin(await cg.gpio_pin_expression(config[CONF_MISO_PIN])))
    cg.add(var.set_mosi_pin(await cg.gpio_pin_expression(config[CONF_MOSI_PIN])))
    cg.add(var.set_cs_pin(await cg.gpio_pin_expression(config[CONF_CS_PIN])))
    cg.add(var.set_clk_pin(await cg.gpio_pin_expression(config[CONF_CLK_PIN])))
    cg.add(var.set_base_url(config[CONF_BASE_URL]))
    cg.add(var.set_uuid(str(config[CONF_UUID])))
    await cg.register_component(var, config)

CSMN_KEYPOINTS_ACTION_SCHEMA = maybe_simple_id({cv.GenerateID(): cv.use_id(CsmnKeyPoints)})

@automation.register_action(
    "csmn_keypoints.start", StartAction, CSMN_KEYPOINTS_ACTION_SCHEMA
)
@automation.register_action(
    "csmn_keypoints.stop", StopAction, CSMN_KEYPOINTS_ACTION_SCHEMA
)
@automation.register_action(
    "csmn_keypoints.toggle", ToggleAction, CSMN_KEYPOINTS_ACTION_SCHEMA
)
async def csmn_keypoints_action(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

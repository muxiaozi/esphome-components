import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch, uart
from esphome.const import (
    CONF_ID, 
    CONF_ADDRESS, 
    CONF_NUM_CHANNELS, 
    CONF_CHANNEL, 
    CONF_NAME
)

CODEOWNERS = ["hongwei.hu@foxmail.com"]
DEPENDENCIES = ["uart", "switch"]
AUTO_LOAD = ["switch"]

zhiyuan_relay_ns = cg.esphome_ns.namespace("zhiyuan_relay")
ZhiyuanRelay = zhiyuan_relay_ns.class_("ZhiyuanRelay", uart.UARTDevice, cg.Component)
RelayChannel = zhiyuan_relay_ns.class_("RelayChannel", switch.Switch)

CHANNEL_SCHEMA = switch.switch_schema(RelayChannel).extend({
    cv.Required(CONF_CHANNEL): cv.positive_not_null_int,
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.declare_id(ZhiyuanRelay),
    cv.Required(CONF_NUM_CHANNELS): cv.int_range(min=1, max=32),
    cv.Optional(CONF_ADDRESS, default=1): cv.positive_int,
}).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    parent = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(parent, config)
    await uart.register_uart_device(parent, config)
    cg.add(parent.set_address(config[CONF_ADDRESS]))

    for i in range(config[CONF_NUM_CHANNELS]):
        channel_config = {
            CONF_ID: cv.declare_id(RelayChannel)(f"{config[CONF_ID]}_ch{i+1}"),
            CONF_NAME: f"Switch {i+1}",
            CONF_CHANNEL: i + 1,
        }
        channel_config = CHANNEL_SCHEMA(channel_config)
        channel = await switch.new_switch(channel_config, i + 1)
        await cg.register_parented(channel, parent)
        cg.add(parent.add_channel(channel))


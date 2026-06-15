import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "binary_sensor"]

CONF_INITIALIZE_ON_BOOT = "initialize_on_boot"
CONF_RESET_EVENTS_EACH_CYCLE = "reset_events_each_cycle"
CONF_AXIS_MODE = "axis_mode"

AXIS_MODES = {
    "force_yz": 0x00,
    "force_xz": 0x01,
    "force_xy": 0x02,
    "auto_switch": 0x03,
    "switch_at_installation": 0x04,
}

d7s_ns = cg.esphome_ns.namespace("d7s")
D7SComponent = d7s_ns.class_("D7SComponent", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(D7SComponent),
            cv.Optional(CONF_INITIALIZE_ON_BOOT, default=False): cv.boolean,
            cv.Optional(CONF_RESET_EVENTS_EACH_CYCLE, default=True): cv.boolean,
            cv.Optional(CONF_AXIS_MODE, default="switch_at_installation"): cv.enum(
                AXIS_MODES, lower=True, space="_"
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(0x55))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_initialize_on_boot(config[CONF_INITIALIZE_ON_BOOT]))
    cg.add(var.set_reset_events_each_cycle(config[CONF_RESET_EVENTS_EACH_CYCLE]))
    cg.add(var.set_axis_mode(config[CONF_AXIS_MODE]))

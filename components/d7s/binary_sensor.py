import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import D7SComponent

CONF_D7S_ID = "d7s_id"
CONF_EARTHQUAKE_OCCURRING = "earthquake_occurring"
CONF_COLLAPSE_OCCURRING = "collapse_occurring"
CONF_SHUTOFF_OCCURRING = "shutoff_occurring"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_D7S_ID): cv.use_id(D7SComponent),
        cv.Optional(CONF_EARTHQUAKE_OCCURRING): binary_sensor.binary_sensor_schema(
            device_class="problem",
            entity_category="diagnostic",
        ),
        cv.Optional(CONF_COLLAPSE_OCCURRING): binary_sensor.binary_sensor_schema(
            device_class="problem",
            entity_category="diagnostic",
        ),
        cv.Optional(CONF_SHUTOFF_OCCURRING): binary_sensor.binary_sensor_schema(
            device_class="problem",
            entity_category="diagnostic",
        ),
    }
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_D7S_ID])

    if CONF_EARTHQUAKE_OCCURRING in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_EARTHQUAKE_OCCURRING])
        cg.add(hub.set_earthquake_occurring_binary_sensor(sens))

    if CONF_COLLAPSE_OCCURRING in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_COLLAPSE_OCCURRING])
        cg.add(hub.set_collapse_occurring_binary_sensor(sens))

    if CONF_SHUTOFF_OCCURRING in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_SHUTOFF_OCCURRING])
        cg.add(hub.set_shutoff_occurring_binary_sensor(sens))

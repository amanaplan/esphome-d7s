import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from . import D7SComponent

CONF_D7S_ID = "d7s_id"
CONF_INSTANTANEOUS_SI = "instantaneous_si"
CONF_INSTANTANEOUS_PGA = "instantaneous_pga"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_D7S_ID): cv.use_id(D7SComponent),
        cv.Optional(CONF_INSTANTANEOUS_SI): sensor.sensor_schema(
            unit_of_measurement="m/s",
            accuracy_decimals=3,
            state_class="measurement",
            entity_category="diagnostic",
            icon="mdi:chart-line",
        ),
        cv.Optional(CONF_INSTANTANEOUS_PGA): sensor.sensor_schema(
            unit_of_measurement="m/s²",
            accuracy_decimals=3,
            state_class="measurement",
            entity_category="diagnostic",
            icon="mdi:axis-arrow",
        ),
    }
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_D7S_ID])

    if CONF_INSTANTANEOUS_SI in config:
        sens = await sensor.new_sensor(config[CONF_INSTANTANEOUS_SI])
        cg.add(hub.set_instantaneous_si_sensor(sens))

    if CONF_INSTANTANEOUS_PGA in config:
        sens = await sensor.new_sensor(config[CONF_INSTANTANEOUS_PGA])
        cg.add(hub.set_instantaneous_pga_sensor(sens))

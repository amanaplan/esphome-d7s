import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_ACCELERATION,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_METER_PER_SECOND_SQUARED,
)

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
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_INSTANTANEOUS_PGA): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ACCELERATION,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
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

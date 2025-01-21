import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_VOLATILE_ORGANIC_COMPOUNDS_PARTS,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_BILLION,
    CONF_TVOC,
)

AUTO_LOAD = ["sensor"]
DEPENDENCIES = ["i2c"]

ags02ma_ns = cg.esphome_ns.namespace("ags02ma")
AGS02MAComponent = ags02ma_ns.class_(
    "AGS02MAComponent", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AGS02MAComponent),
            cv.Required(CONF_TVOC): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_BILLION,
                icon="mdi:molecule",
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLATILE_ORGANIC_COMPOUNDS_PARTS,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x1A))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    sens = await sensor.new_sensor(config[CONF_TVOC])
    cg.add(var.set_tvoc(sens))

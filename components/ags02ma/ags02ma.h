#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome
{
    namespace ags02ma
    {

        class AGS02MAComponent : public PollingComponent, public i2c::I2CDevice
        {
        public:
            void set_tvoc(sensor::Sensor *tvoc) { tvoc_ = tvoc; }

            void setup() override;
            void update() override;

            void dump_config() override;

            float get_setup_priority() const override { return setup_priority::DATA; }

        protected:
            uint8_t calc_CRC8(uint8_t *dat, uint8_t Num);

            enum ErrorCode
            {
                UNKNOWN,
                COMMUNICATION_FAILED,
                NO_VERSION,
                NO_DATA,
                CRC8_FAILED,
                SENSOR_REPORTED_ERROR,
            } error_code_{UNKNOWN};

            sensor::Sensor *tvoc_{nullptr};
            optional<uint16_t> baseline_{};
        };

    }
}
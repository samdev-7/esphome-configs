#include "ags02ma.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#define CHECK_TRUE(f, error_code)         \
    if (!(f))                             \
    {                                     \
        this->mark_failed();              \
        this->error_code_ = (error_code); \
        return;                           \
    }

#define CHECKED_IO(f) CHECK_TRUE(f, COMMUNICATION_FAILED)

#define AGS02MA_VERSION 0x11
#define AGS02MA_DATA_COLLECTION 0x00

namespace esphome
{
    namespace ags02ma
    {
        static const char *const TAG = "ags02ma";

        void AGS02MAComponent::setup()
        {
            auto hw_version_data = this->read_bytes<10>(AGS02MA_VERSION);
            uint8_t hw_version = 0;

            if (!hw_version_data.has_value())
            {
                this->mark_failed();
                this->error_code_ = NO_VERSION;
                return;
            }

            hw_version = (*hw_version_data)[3];
            ESP_LOGD(TAG, "AGS02MA Hardware Version: %d", hw_version);
        }
        void AGS02MAComponent::update()
        {
            auto data = this->read_bytes<5>(AGS02MA_DATA_COLLECTION);

            if (!data.has_value())
            {
                this->mark_failed();
                this->error_code_ = NO_DATA;
                return;
            }

            bool data_okay = !((*data)[0] & 0x01);
            CHECK_TRUE(data_okay, SENSOR_REPORTED_ERROR);

            uint32_t tvoc = (*data)[1] * 65536UL;
            tvoc += (*data)[2] * 256;
            tvoc += (*data)[3];

            ESP_LOGD(TAG, "Got TVOC=%d ppb", tvoc);

            if (this->calc_CRC8((*data).data(), 5) != 0)
            {
                // this->mark_failed();
                // this->error_code_ = CRC8_FAILED;
                ESP_LOGW(TAG, "CRC8 check failed");
                return;
            }

            if (this->tvoc_ != nullptr)
            {
                this->tvoc_->publish_state(tvoc);
            }

            this->status_clear_warning();
        }
        void AGS02MAComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "AGS02MA");
            LOG_I2C_DEVICE(this);
            LOG_UPDATE_INTERVAL(this);
            LOG_SENSOR("  ", "TVOC Sensor", this->tvoc_);
            LOG_UPDATE_INTERVAL(this);
            if (this->is_failed())
            {
                switch (this->error_code_)
                {
                case COMMUNICATION_FAILED:
                    ESP_LOGE(TAG, "Communication failed! Is the sensor connected?");
                    break;
                case NO_VERSION:
                    ESP_LOGE(TAG, "Failed to read hardware version");
                    break;
                case NO_DATA:
                    ESP_LOGE(TAG, "Failed to read data");
                    break;
                case CRC8_FAILED:
                    ESP_LOGE(TAG, "CRC8 check failed");
                    break;
                case SENSOR_REPORTED_ERROR:
                    ESP_LOGE(TAG, "Sensor reported internal error");
                    break;
                case UNKNOWN:
                default:
                    ESP_LOGE(TAG, "Unknown error");
                    break;
                }
            }
        }

        uint8_t AGS02MAComponent::calc_CRC8(uint8_t *dat, uint8_t Num)
        {
            uint8_t i, byte, crc = 0xFF;
            for (byte = 0; byte < Num; byte++)
            {
                crc ^= (dat[byte]);
                for (i = 0; i < 8; i++)
                {
                    if (crc & 0x80)
                        crc = (crc << 1) ^ 0x31;
                    else
                        crc = (crc << 1);
                }
            }
            return crc;
        }
    }
}
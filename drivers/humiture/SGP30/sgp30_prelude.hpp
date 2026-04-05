#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

//参考资料：
// https://github.com/adafruit/Adafruit_SGP30/blob/master/Adafruit_SGP30.cpp

namespace ymd::drivers{

struct SGP30_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x58);


    enum class [[nodiscard]] Error_Kind:uint8_t{
        ChipIdMismatch
    };
    
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
    

    // https://github.com/adafruit/Adafruit_SGP30/blob/master/Adafruit_SGP30.cpp

    /* return absolute humidity [mg/m^3] with approximation formula
    * @param temperature [°C]
    * @param humidity [%RH]
    */
    uint32_t getAbsoluteHumidity(float temperature, float humidity) {
        // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
        const float absolute_humidity = 216.7f * ((humidity / 100.0f) * 6.112f * std::exp(
            (17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]

        const uint32_t absolute_humidity_scaled = static_cast<uint32_t>(1000.0f * absolute_humidity); // [mg/m^3]
        return absolute_humidity_scaled;
    }

    // commands and constants
    static constexpr auto SGP30_FEATURESET = 0x0020;    ///< The required set for this library
    static constexpr auto SGP30_CRC8_POLYNOMIAL = 0x31; ///< Seed for SGP30's CRC polynomial
    static constexpr auto SGP30_CRC8_INIT = 0xFF;       ///< Init value for CRC
    static constexpr auto SGP30_WORD_LEN = 2;           ///< 2 bytes per word

    struct [[nodiscard]] ChecksumBuilder final{
        uint8_t checksum;

        static constexpr ChecksumBuilder from_default(){
            return ChecksumBuilder{.checksum = SGP30_CRC8_INIT};
        }

    
        constexpr ChecksumBuilder push_byte(const uint8_t byte) const {
            ChecksumBuilder self = *this;
            self.checksum ^= byte;
            for (uint8_t b = 0; b < 8; b++) {
                if (self.checksum & 0x80)
                    self.checksum = (self.checksum << 1) ^ SGP30_CRC8_POLYNOMIAL;
                else
                    self.checksum <<= 1;
            }
            return self;
        }

        constexpr ChecksumBuilder push_bytes(std::span<const uint8_t> bytes) const {
            ChecksumBuilder self = *this;
            for(size_t i = 0; i < bytes.size(); i++){
                self = self.push_byte(bytes[i]);
            }
            return self;
        }

        [[nodiscard]] uint8_t finalize() const {
            return checksum;
        }
    };
};

}

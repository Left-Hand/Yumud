#pragma once

#include <tuple>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/ms5611-pressure-sensor.html

namespace ymd::drivers{

struct MS5611_Prelude{
static constexpr auto I2C_DEFAULT_ADDR = hal::I2cSlaveAddr<7>::from_u7(0xee >> 1);
static constexpr uint8_t D1_ADDR = 0x48;
static constexpr uint8_t D2_ADDR = 0x58;
static constexpr uint8_t RESET_COMMAND = 0x1e;

struct [[nodiscard]] Coeffs final{
    uint16_t c_table[6];

    struct [[nodiscard]] Intermediate final{
        uint32_t d1;
        int32_t dt;
        int32_t temp;
    };

    constexpr Intermediate calc_intermediate(const uint32_t d1, const uint32_t d2) const{
        const int32_t dt = d2 - (c_table[4] * 256);

        const int32_t TEMP = 2000 + ((static_cast<int64_t>(dt) * static_cast<int64_t>(c_table[5])) >> 23);
        return Intermediate{
            .d1 = d1,
            .dt = dt,
            .temp = TEMP
        };
    }
    
    struct Product{
        int64_t off;
        int64_t sens;
        int32_t p;
    };

    constexpr Product calc_product(const Intermediate intermediate) const {
        const int64_t off = (static_cast<uint32_t>(c_table[1]) << 16) + ((static_cast<int64_t>(c_table[3]) * static_cast<int64_t>(intermediate.dt)) >> 7);
        const int64_t sens = (static_cast<uint32_t>(c_table[0]) << 15) + ((c_table[2] * intermediate.dt ) >> 8);

        const int32_t P = ((intermediate.d1 * sens >> 21) - off) >> 15;
        return Product{
            .off = off,
            .sens = sens,
            .p = P
        };
    }
};


};

}
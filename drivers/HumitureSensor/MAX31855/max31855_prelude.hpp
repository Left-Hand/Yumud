#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"

#include "concept/analog_channel.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct MAX31855_Prelude{
    static constexpr real_t JUNC_TEMP_LSB = 0.0625_r;
    static constexpr real_t THER_TEMP_LSB = 0.25_r;

    struct MAX31855_Payload{
        // D0 OC Fault This bit is a 1 when the thermocouple is open (no connections). Default value is 0.
        uint32_t oc_fault:1;

        // D1 SCG Fault This bit is a 1 when the thermocouple is short-circuited to GND. Default value is 0.
        uint32_t scg_fault:1;

        // D2 SCV Fault This bit is a 1 when the thermocouple is short-circuited to VCC. Default value is 0.
        uint32_t scv_fault:1;

        uint32_t __resv__:1;
        uint32_t junc_temp:12;
        uint32_t any_fault:1;
        uint32_t __resv2__:1;
        uint32_t ther_temp:14;

        real_t ther_temperature() const{
            const real_t uns_ther_temp = (ther_temp & ((1 << 13) - 1)) * THER_TEMP_LSB;
            const bool is_negative = (ther_temp & (1 << 13)) != 0;
            return is_negative ? (-uns_ther_temp) : uns_ther_temp;
        }

        real_t junc_temperature() const{
            const real_t uns_junc_temp = (junc_temp & ((1 << 11) - 1)) * JUNC_TEMP_LSB;
            const bool is_negative = (junc_temp & (1 << 11)) != 0;
            return is_negative ? (-uns_junc_temp) : uns_junc_temp;
        }

        std::span<uint16_t, 2> as_u16_slice(){
            static_assert(sizeof(MAX31855_Payload) == 4);
            return std::span<uint16_t, 2>(reinterpret_cast<uint16_t *>(this), 2);
        }
    };

    static_assert(sizeof(MAX31855_Payload) == 4);

    using IResult = Result<MAX31855_Payload, hal::HalError>;
};


}
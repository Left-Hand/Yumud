#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"

#include "concept/analog_channel.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct MAX31855_Collections{
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
            const bool is_minus = (ther_temp & (1 << 13)) != 0;
            return is_minus ? (-uns_ther_temp) : uns_ther_temp;
        }

        real_t junc_temperature() const{
            const real_t uns_junc_temp = (junc_temp & ((1 << 11) - 1)) * JUNC_TEMP_LSB;
            const bool is_minus = (junc_temp & (1 << 11)) != 0;
            return is_minus ? (-uns_junc_temp) : uns_junc_temp;
        }

        std::span<uint16_t> to_u16_span(){
            return std::span<uint16_t>(reinterpret_cast<uint16_t *>(this), 2);
        }
    };

    static_assert(sizeof(MAX31855_Payload) == 4);

    using MAX31855_Result = Result<MAX31855_Payload, hal::HalError>;
};

class MAX31855 final:public MAX31855_Collections{
public:
    MAX31855(hal::SpiDrv && spi_drv):   
        spi_drv_(std::move(spi_drv)){;}

    MAX31855_Result read(){
        MAX31855_Payload payload;
        // if(const auto res = spi_drv_.)
        const auto raw_span = payload.to_u16_span();
        if(const auto res = spi_drv_.read_burst<uint16_t>(
                std::span(raw_span)
            );
            res.is_err()) return Err(res.unwrap_err());
        return Ok<MAX31855_Payload>(payload);
    }
private:
    hal::SpiDrv spi_drv_;
};

}
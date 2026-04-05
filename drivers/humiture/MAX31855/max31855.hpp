#pragma once

#include "mlx31855_prelude.hpp"


namespace ymd::drivers{


class MAX31855 final:public MAX31855_Prelude{
public:
    explicit MAX31855(hal::SpiDrv && spi_drv):   
        spi_drv_(std::move(spi_drv)){;}

    IResult read(){
        MAX31855_Payload payload;
        // if(const auto res = spi_drv_.)
        const auto raw_span = payload.to_u16_slice();
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
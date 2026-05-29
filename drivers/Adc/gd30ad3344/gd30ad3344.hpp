#pragma once

#include "gd30ad3344_prelude.hpp"

// 这个驱动已完成
// 这个驱动未测试

namespace ymd::drivers{


class GD30AD3344 final:
    public GD30AD3344_Prelude{
public:

    explicit GD30AD3344(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit GD30AD3344(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}

    GD30AD3344(const GD30AD3344 &) = delete;
    GD30AD3344(GD30AD3344 &&) = delete;
    ~GD30AD3344() = default;


    Result<ConvCode, Error> enquery(const ConfCode conf){
        uint16_t rx;
        if(const auto res = transeceive_u16(rx, conf.to_u16());
            res.is_err()) return Err(res.unwrap_err());
        return Ok(ConvCode{.bits = std::bit_cast<int16_t>(rx)});
    }
private:

    Result<void, Error> transeceive_u16(uint16_t & rx, uint16_t tx){
        if(const auto res = spi_drv_.transceive_single(rx, tx);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    hal::SpiDrv spi_drv_;
};

}
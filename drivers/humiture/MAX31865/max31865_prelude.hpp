#pragma once


//  MAX31865是简单易用的热敏电阻至数字输出转换器，优化用于铂电阻温度传感器(RTD)。
// 外部电阻设置所用RTD的灵敏度，高精度Σ-Δ ADC将RTD电阻与参考阻值之比转换成数字输出。
// MAX31865输入具有高达±45V的过压保护，提供可配置的RTD及电缆开路/短路条件检测

// 参考资料：https://blog.csdn.net/qq_42250136/article/details/156201848

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"

#include "hal/conn/spi/spidrv.hpp"
#include "core/math/real.hpp"

namespace ymd::drivers{


struct MAX31865_Prelude{
    using Error = hal::HalError;

    template<typename T = void>
    using IResult = Result<T, Error>;
};


class MAX31865 final:public MAX31865_Prelude{
public:
    explicit MAX31865(hal::SpiDrv && spi_drv):   
        spi_drv_(std::move(spi_drv)){;}

private:
    hal::SpiDrv spi_drv_;

    IResult<> write(const uint8_t addr, const uint8_t val){
        return write_bulk(addr, std::span(&val, 1));
    }

    IResult<> read(const uint8_t addr, uint8_t & val){
        return read_bulk(addr, std::span(&val, 1));
    }

    IResult<> write_bulk(const uint8_t addr, std::span<const uint8_t> data){
        if(const auto res = spi_drv_.write_single<uint8_t>(addr, CONT);
            res.is_err()) return Err(res.unwrap_err());

        if(const auto res = spi_drv_.write_bulk<uint8_t>(data);
            res.is_err()) return Err(res.unwrap_err());
        
        return Ok();
    }

    IResult<> read_bulk(const uint8_t addr, std::span<uint8_t> data){
        if(const auto res = spi_drv_.write_single<uint8_t>(addr, CONT);
            res.is_err()) return Err(res.unwrap_err());

        if(const auto res = spi_drv_.read_bulk<uint8_t>(data);
            res.is_err()) return Err(res.unwrap_err());
        
        return Ok();
    }
};

}
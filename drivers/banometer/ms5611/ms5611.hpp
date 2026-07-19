#pragma once

#include "ms5611_prelude.hpp"

namespace ymd::drivers{


class MS5611_Transport final:public MS5611_Prelude{
public:
    explicit MS5611_Transport(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    explicit MS5611_Transport(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}


    IResult<> write_reg(const uint8_t reg_addr, const uint8_t reg_val){
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const uint8_t reg_addr, uint8_t & reg_val){
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_bulk(const uint8_t reg_addr, const std::span<int16_t> pbuf){
        if(const auto res = i2c_drv_.read_bulk(uint8_t(reg_addr), pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_u24(uint32_t & val);

private:
    hal::I2cDrv i2c_drv_;
};
class MS5611 final:public MS5611_Prelude{
public:

    explicit MS5611(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv(i2c, i2c_addr)){;}

    ~MS5611(){;}

    struct Config{
    };

    IResult<> init(const Config & cfg);

private:
    MS5611_Transport transport_;
    Coeffs coeffs_;
};


}
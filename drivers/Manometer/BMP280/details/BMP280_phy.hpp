#pragma once
#include "BMP280_Prelude.hpp"

namespace ymd::drivers{
class BMP280_Phy final:public BMP280_Prelude{
public:
    BMP280_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    BMP280_Phy(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}


    IResult<> write_reg(const RegAddress addr, const uint16_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddress addr, uint16_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(const RegAddress addr, const uint8_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddress addr, uint8_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
        // BMP280_DEBUG(uint8_t(addr), (uint8_t)data);
    }

    IResult<> read_burst(const RegAddress addr, const std::span<int16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf, MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

private:
    hal::I2cDrv i2c_drv_;
};
}
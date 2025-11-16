#pragma once
#include "BMP280_Prelude.hpp"

namespace ymd::drivers{
class BMP280_Phy final:public BMP280_Prelude{
public:
    explicit BMP280_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    explicit BMP280_Phy(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}


    IResult<> write_reg(const RegAddr reg_addr, const uint16_t reg_val){
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddr reg_addr, uint16_t & reg_val){
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(const RegAddr reg_addr, const uint8_t reg_val){
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddr reg_addr, uint8_t & reg_val){
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
        // BMP280_DEBUG(uint8_t(reg_addr), (uint8_t)reg_val);
    }

    IResult<> read_burst(const RegAddr reg_addr, const std::span<int16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(reg_addr), pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

private:
    hal::I2cDrv i2c_drv_;
};
}
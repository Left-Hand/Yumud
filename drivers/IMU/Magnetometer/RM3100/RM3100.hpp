#pragma once

#include "rm3100_prelude.hpp"


namespace ymd::drivers{



struct RM3100_I2cTransport:public RM3100_Prelude{
    explicit RM3100_I2cTransport(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){}

    IResult<> write_reg(RegAddr reg_addr, uint8_t reg_val){
        if(const auto res = i2c_drv_.write_reg(static_cast<uint8_t>(reg_addr), reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(RegAddr reg_addr, uint8_t & reg_val){
        if(const auto res = i2c_drv_.read_reg(static_cast<uint8_t>(reg_addr), reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_bulk(RegAddr reg_addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_bulk<uint8_t>(static_cast<uint8_t>(reg_addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
private:
    hal::I2cDrv i2c_drv_;
};

struct RM3100:public RM3100_Prelude{
    explicit RM3100(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv(i2c, i2c_addr)){}


    IResult<bool> is_conv_done(){
        uint8_t status;
        if(const auto res = transport_.read_reg(RegAddr::STATUS, status);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(status & 0x80);
    }

    IResult<math::Vec3<int32_t>> get_mag_i32();
private:
    RM3100_I2cTransport transport_;
};

}

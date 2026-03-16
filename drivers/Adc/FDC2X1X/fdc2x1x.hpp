#pragma once

#include "fdc2x1x_prelude.hpp"

namespace ymd::drivers{


class FDC2X1X final:public FDC1X2X_Regs{
public:
    explicit FDC2X1X(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit FDC2X1X(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit FDC2X1X(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){};


    IResult<> init();

    IResult<bool> is_conv_done();

    IResult<bool> is_conv_done(uint8_t idx);

    IResult<> reset();

    IResult<uint32_t> get_data(uint8_t idx);

private:
    hal::I2cDrv i2c_drv_;

    IResult<> read_reg(const RegAddr reg_addr, uint16_t & reg_val){
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(const RegAddr reg_addr, const uint16_t reg_val){
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
};

}
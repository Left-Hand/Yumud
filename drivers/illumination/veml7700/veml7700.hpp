#pragma once

#include "veml7700_prelude.hpp"


// VEML7700 is a high accuracy ambient light digital 16-bit
// resolution sensor in a miniature transparent 6.8 mm x
// 2.35 mm x 3.0 mm package. It includes a high sensitive
// photo diode, a low noise amplifier, a 16-bit A/D converter
// and supports an easy to use I2C bus communication
// interface.
// The ambient light result is as digital value available

namespace ymd::drivers{

struct VEML7700:public VEML7700_Prelude{
    explicit VEML7700(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit VEML7700(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit VEML7700(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, i2c_addr){;}

    VEML7700(const VEML7700 &) = delete;
    VEML7700(VEML7700 &&) = delete;
    ~VEML7700() = default;
private:
    hal::I2cDrv i2c_drv_;

    using Regs = VEML7700_Regset;
    Regs regs_ = {};


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            uint8_t(T::REG_ADDR), 
            reg.to_bits(), std::endian::little
        ); res.is_err()) return Err(res.unwrap_err());
        reg.commit_changes();
        return Ok();
    }
    

    IResult<> read_bulk(const RegAddr reg_addr, const std::span<uint16_t> pbuf);
};


}
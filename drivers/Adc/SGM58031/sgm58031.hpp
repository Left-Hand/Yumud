#pragma once

#include "sgm58031_prelude.hpp"

namespace ymd::drivers{


class SGM58031 final:public SGM58031_Prelude{
public:
    explicit SGM58031(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit SGM58031(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit SGM58031(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){};

    IResult<> init();
    IResult<> validate();
    IResult<bool> is_idle();
    IResult<> start_conv();
    IResult<int16_t> get_conv_data();
    IResult<iq16> get_conv_voltage();
    IResult<> enable_cont_mode(const Enable en);
    IResult<> set_datarate(const DataRate dr);
    IResult<> set_mux(const MUX mux);
    IResult<> set_fs(const FS fs);
    IResult<> set_fs(const uq16 fs, const uq16 vref);
    IResult<> set_trim(const iq16 trim);
    IResult<> enable_ch3_as_bits_mut(const Enable en);
private:
    using Regs = SGM58031_Regset;
    hal::I2cDrv i2c_drv_;
    Regs regs_ = {};
    iq16 full_scale_ = 0;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            uint8_t(T::REG_ADDR), reg.to_bits(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            uint8_t(T::REG_ADDR), reg.as_bits_mut(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


};

}
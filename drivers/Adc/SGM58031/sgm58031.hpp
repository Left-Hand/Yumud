#pragma once

#include "sgm58031_prelude.hpp"

namespace ymd::drivers{


class SGM58031 final:public SGM58031_Prelude{
public:
    explicit SGM58031(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit SGM58031(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit SGM58031(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<bool> is_idle();
    [[nodiscard]] IResult<> start_conv();
    [[nodiscard]] IResult<int16_t> get_conv_data();
    [[nodiscard]] IResult<iq16> get_conv_voltage();
    [[nodiscard]] IResult<> enable_cont_mode(const Enable en);
    [[nodiscard]] IResult<> set_datarate(const DataRate dr);
    [[nodiscard]] IResult<> set_mux(const MUX mux);
    [[nodiscard]] IResult<> set_fs(const FS fs);
    [[nodiscard]] IResult<> set_fs(const uq16 fs, const uq16 vref);
    [[nodiscard]] IResult<> set_trim(const iq16 trim);
    [[nodiscard]] IResult<> enable_ch3_as_mut_bits(const Enable en);
private:
    hal::I2cDrv i2c_drv_;
    SGM58031_Regset regs_ = {};
    iq16 full_scale_ = 0;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            uint8_t(T::ADDRESS), reg.as_bits(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            uint8_t(T::ADDRESS), reg.as_mut_bits(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


};

}
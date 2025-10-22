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

    IResult<> init();
    IResult<> validate();
    IResult<bool> is_idle();
    IResult<> start_conv();
    IResult<int16_t> get_conv_data();
    IResult<q16> get_conv_voltage();
    IResult<> enable_cont_mode(const Enable en);
    IResult<> set_datarate(const DataRate _dr);
    IResult<> set_mux(const MUX _mux);
    IResult<> set_fs(const FS fs);
    IResult<> set_fs(const q16 fs, const q16 vref);
    IResult<> set_trim(const q16 trim);
    IResult<> enable_ch3_as_ref(const Enable en);
private:
    hal::I2cDrv i2c_drv_;
    SGM58031_Regset regs_ = {};
    q16 full_scale_ = 0;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(
            uint8_t(T::ADDRESS), reg.as_val(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(
            uint8_t(T::ADDRESS), reg.as_ref(), MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    static constexpr PGA ratio2pga(const q16 ratio){
        if(ratio >= 3){
            return PGA::_2_3;
        }else if(ratio >= 2){
            return PGA::_1;
        }else if(ratio >= 1){
            return PGA::_2;
        }else if(ratio >= q16(0.5)){
            return PGA::_4;
        }else if(ratio >= q16(0.25)){
            return PGA::_8;
        }else{
            return PGA::_16;
        }
    }

};

}
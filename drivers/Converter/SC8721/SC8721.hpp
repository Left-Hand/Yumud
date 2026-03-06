#pragma once

#include "sc8721_prelude.hpp"
namespace ymd::drivers{


class SC8721 final:public SC8721_Prelude{
public:
    explicit SC8721(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit SC8721(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit SC8721(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    IResult<> update();

    IResult<> validate();

    IResult<> reset();

    IResult<> set_target_voltage(const iq16 volt);

    IResult<> enable_external_fb(const Enable en);

    IResult<> set_dead_zone(const DeadZone dz);

    IResult<> set_switch_freq(const SwitchFreq freq);

    IResult<Status> get_status();

    IResult<> set_slope_comp(const SlopComp sc);
private:
    hal::I2cDrv i2c_drv_;
    SC8721_Regs regs_;
    IResult<> write_reg(const RegAddr address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddr address, uint8_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_burst(const RegAddr addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
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

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }
};

}
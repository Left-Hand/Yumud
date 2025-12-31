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

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_target_voltage(const real_t volt);

    [[nodiscard]] IResult<> enable_external_fb(const Enable en);

    [[nodiscard]] IResult<> set_dead_zone(const DeadZone dz);

    [[nodiscard]] IResult<> set_switch_freq(const SwitchFreq freq);

    [[nodiscard]] IResult<Status> get_status();

    [[nodiscard]] IResult<> set_slope_comp(const SlopComp sc);
private:
    hal::I2cDrv i2c_drv_;
    SC8721_Regs regs_;
    [[nodiscard]] IResult<> write_reg(const RegAddr address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddr address, uint8_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddr addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_bits_mut());
    }
};

}
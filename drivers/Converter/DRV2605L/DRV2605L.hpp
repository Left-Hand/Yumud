#pragma once

#include "drv2605l_prelude.hpp"

namespace ymd::drivers{



class DRV2605L:public DRV2605L_Regs{
public:
    explicit DRV2605L(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit DRV2605L(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit DRV2605L(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> init();
    Package get_package();
    [[nodiscard]] IResult<> set_bemf_gain(const BemfGain gain);
    [[nodiscard]] IResult<> set_loop_gain(const LoopGain gain);
    [[nodiscard]] IResult<> play(const uint8_t idx);
    [[nodiscard]] IResult<> autocal();

private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> 
    write_reg(const RegAddr address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg<uint8_t>(uint8_t(address), reg);
            res.is_err()) return Err(Error(res.unwrap_err()));

        return Ok();
    }

    [[nodiscard]] IResult<> 
    read_reg(const RegAddr addr, uint8_t & reg){
        const auto res = i2c_drv_.read_reg<uint8_t>(uint8_t(addr), reg);
        if(res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok();
    }

    [[nodiscard]] IResult<> 
    requestBurst(const RegAddr addr, uint8_t * data, size_t len){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), std::span(data, len));
            res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_bits());
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }
    [[nodiscard]] IResult<> read_reg(auto & reg){
        return read_reg(reg.address, reg.as_mut_bits());
    }


    [[nodiscard]] IResult<> set_fb_brake_factor(const FbBrakeFactor factor);
    [[nodiscard]] IResult<> set_fb_brake_factor(const int fractor);
};

}
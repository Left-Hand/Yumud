#pragma once

#include "mp2980_prelude.hpp"

namespace ymd::drivers{

class MP2980 : public MP2980_Prelude{
public:
    explicit MP2980(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit MP2980(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit MP2980(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> set_feed_back_vref(const uq10 vref);
    [[nodiscard]] IResult<> set_feed_back_vref_mv(const uint32_t vref_mv);
    [[nodiscard]] IResult<> enable_power_switching(const Enable en);
    [[nodiscard]] IResult<> enable_vref_change_func(const Enable en);
    [[nodiscard]] IResult<> set_png_state(const bool state);
    [[nodiscard]] IResult<> enable_dither(const Enable en);
    [[nodiscard]] IResult<> set_vref_slew_rate(const VrefSlewRate slewrate);
    [[nodiscard]] IResult<> set_ovp_mode(const OvpMode mode);
    [[nodiscard]] IResult<> set_ocp_mode(const OcpMode mode);
    [[nodiscard]] IResult<> set_fsw(const Fsw fsw);
    [[nodiscard]] IResult<> set_buck_boost_fsw(const BuckBoostFsw fsw);
    [[nodiscard]] IResult<> set_curr_limit_threshold(const CurrLimitThreshold threshold);
    [[nodiscard]] IResult<Interrupts> interrupts();
    [[nodiscard]] IResult<> set_interrupt_mask(const Interrupts mask);
    [[nodiscard]] IResult<> set_output_volt(const uq10 output_volt);
    [[nodiscard]] IResult<> init();
private:
    hal::I2cDrv i2c_drv_;
    MP2980_Regs regs_;
    uint32_t fb_up_res_ohms = 90.9 * 1000;
    uint32_t fb_down_res_ohms = 10 * 1000;


    [[nodiscard]] IResult<> write_reg(const RegAddr address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddr address, uint8_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.as_bits_mut());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

};

}
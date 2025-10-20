#pragma once

#include "as5600_prelude.hpp"
namespace ymd::drivers{

class AS5600 final: 
    public MagEncoderIntf,
    public AS5600_Regs{
public:
    AS5600(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    AS5600(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    AS5600(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}
            
    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> set_power_mode(const PowerMode _power_mode);

    [[nodiscard]] IResult<> set_fast_filter(const FastFilter _fast_filter);

    [[nodiscard]] IResult<> set_slow_filter(const SlowFilter _slow_filter);

    [[nodiscard]] IResult<> set_pwm_frequency(const PwmFrequency _pwm_frequency);

    [[nodiscard]] IResult<> set_ouput_stage(const OutputStage _output_stage);

    [[nodiscard]] IResult<> set_hysteresis(const Hysteresis _hysteresis);
    
    [[nodiscard]] IResult<MagStatus> get_mag_status();
    
    [[nodiscard]] IResult<uint8_t> get_gain();
    
    [[nodiscard]] IResult<uint16_t> get_magnitude();
    
    [[nodiscard]] IResult<real_t> get_raw_position();
    
    [[nodiscard]] IResult<real_t> get_position();
    
    [[nodiscard]] IResult<> set_start_position(const real_t position);
    
    [[nodiscard]] IResult<> set_end_position(const real_t position);
    
    [[nodiscard]] IResult<> set_amount_position(const real_t position);
    
    [[nodiscard]] IResult<uint8_t> get_program_times();
    
    [[nodiscard]] IResult<> burn_angle();
    
    [[nodiscard]] IResult<> burn_setting();

private:
    hal::I2cDrv i2c_drv_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto err = i2c_drv_.write_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.as_val(), LSB); 
            err.is_err()) return Err(err.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto err = i2c_drv_.read_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.as_ref(), LSB); 
            err.is_err()) return Err(err.unwrap_err());
        return Ok();
    }

};


}
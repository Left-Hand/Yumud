#pragma once

#include "as5600_prelude.hpp"
namespace ymd::drivers{

class AS5600 final: 
    public MagEncoderIntf,
    public AS5600_Regs{
public:
    explicit AS5600(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit AS5600(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    explicit AS5600(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}
            
    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> set_power_mode(const PowerMode power_mode);

    [[nodiscard]] IResult<> set_fast_filter(const FastFilter fast_filter);

    [[nodiscard]] IResult<> set_slow_filter(const SlowFilter slow_filter);

    [[nodiscard]] IResult<> set_pwm_frequency(const PwmFrequency pwm_frequency);

    [[nodiscard]] IResult<> set_ouput_stage(const OutputStage output_stage);

    [[nodiscard]] IResult<> set_hysteresis(const Hysteresis hysteresis);
    
    [[nodiscard]] IResult<MagStatus> get_mag_status();
    
    [[nodiscard]] IResult<uint8_t> get_gain();
    
    [[nodiscard]] IResult<uint16_t> get_magnitude();
    
    [[nodiscard]] IResult<Angular<uq32>> get_raw_angle();
    
    [[nodiscard]] IResult<Angular<uq32>> get_angle();
    
    [[nodiscard]] IResult<> set_start_angle(const Angular<uq32> angle);
    
    [[nodiscard]] IResult<> set_end_angle(const Angular<uq32> angle);
    
    [[nodiscard]] IResult<> set_amount_angle(const Angular<uq32> angle);
    
    [[nodiscard]] IResult<uint8_t> get_program_times();
    
    [[nodiscard]] IResult<> burn_angle();
    
    [[nodiscard]] IResult<> burn_setting();

private:
    hal::I2cDrv i2c_drv_;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto err = i2c_drv_.write_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.to_bits(), std::endian::little); 
            err.is_err()) return Err(err.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto err = i2c_drv_.read_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.as_bits_mut(), std::endian::little); 
            err.is_err()) return Err(err.unwrap_err());
        return Ok();
    }

};


}
#pragma once

#include "as5600_prelude.hpp"
namespace ymd::drivers{

class AS5600 final:public AS5600_Prelude{
public:

    explicit AS5600(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit AS5600(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    explicit AS5600(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, i2c_addr}){;}
            
    IResult<> init();

    IResult<> set_power_mode(const PowerMode power_mode);

    IResult<> set_fast_filter(const FastFilter fast_filter);

    IResult<> set_slow_filter(const SlowFilter slow_filter);

    IResult<> set_pwm_frequency(const PwmFrequency pwm_frequency);

    IResult<> set_ouput_stage(const OutputStage output_stage);

    IResult<> set_hysteresis(const Hysteresis hysteresis);
    
    IResult<EncoderFaultBitFields> get_fault();
    
    IResult<uint8_t> get_gain();
    
    IResult<uint16_t> get_magnitude();
    
    IResult<Angular<uq32>> get_raw_angle();
    
    IResult<Angular<uq32>> get_angle();
    
    IResult<> set_start_angle(const Angular<uq32> angle);
    
    IResult<> set_end_angle(const Angular<uq32> angle);
    
    IResult<> set_amount_angle(const Angular<uq32> angle);
    
    IResult<uint8_t> get_program_times();
    
    IResult<> burn_angle();
    
    IResult<> burn_setting();

private:
    AS5600_Regs regs_ = {};
    hal::I2cDrv i2c_drv_;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto err = i2c_drv_.write_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.to_bits(), std::endian::little); 
            err.is_err()) return Err(err.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto err = i2c_drv_.read_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.as_bits_mut(), std::endian::little); 
            err.is_err()) return Err(err.unwrap_err());
        return Ok();
    }

};


}
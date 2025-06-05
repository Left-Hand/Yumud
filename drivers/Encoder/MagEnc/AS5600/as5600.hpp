#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

namespace ymd::drivers{

struct AS5600_Collections{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x1e);

    enum class PowerMode:uint8_t{
        Norm = 0, Pol5Ms, Pol10Ms, Pol100Ms
    };

    enum class Hysteresis:uint8_t{
        OFF = 0, LSB1, LSB2, LSB3
    };

    enum class OutputStage:uint8_t{
        AnalogRailToRail = 0, AnalogWithBand,DigitalPwm 
    };

    enum class PwmFrequency:uint8_t{
        Hz115, Hz230, Hz460, Hz920
    };

    enum class SlowFilter:uint8_t{
        X16 = 0, X8, X4, X2
    };

    enum class FastFilter:uint8_t{
        None = 0, LSB6, LSB7, LSB9, LSB18, LSB21, LSB24, LSB10
    };

    enum class RegAddress : uint8_t {
        ProgramTimes = 0x00,
        StartAngle = 0x01,
        EndAngle = 0x03,
        AmountAngle = 0x05,
        Config = 0x07,
        RawAngle = 0x0c,
        Angle = 0x0d,
        Status = 0x0B,
        AutoGain = 0x1A,
        Magnitude = 0x1B,
        Burn = 0xFF
    };

};

struct AS5600_Regs:public AS5600_Collections{

    struct R8_ProgramTimes:public Reg8<>{
        static constexpr auto address = RegAddress::ProgramTimes;
        uint8_t times :2;
        uint8_t __resv__ :6;
        
    };

    struct R16_StartAngle:public Reg16<>{
        static constexpr auto address = RegAddress::StartAngle;
        uint16_t data;
    };

    struct R16_EndAngle:public Reg16<>{
        static constexpr auto address = RegAddress::EndAngle;
        uint16_t data;
    };

    struct R16_AmountAngle:public Reg16<>{
        static constexpr auto address = RegAddress::AmountAngle;        
        uint16_t data;
    };

    struct R16_RawAngle:public Reg16<>{
        static constexpr auto address = RegAddress::RawAngle;
        uint16_t data;
    };

    struct R16_Angle:public Reg16<>{
        static constexpr auto address = RegAddress::Angle;
        uint16_t data;
    };

    struct R16_Config:public Reg16<>{
        static constexpr auto address = RegAddress::Config;
        uint8_t powerMode :2;
        uint8_t hysteresis:2;
        uint8_t outputStage:2;
        uint8_t pwmFrequency:2;
        uint8_t slowFilter:2;
        uint8_t fastFilter:3;
        uint8_t watchDog:1;
        uint8_t __resv__ :2;
    };

    struct R8_Status:public Reg8<>{
        static constexpr auto address = RegAddress::Status;
        
        uint8_t __resv1__ :3;
        uint8_t magHigh:1;
        uint8_t magLow:1;
        uint8_t magProper:1;
        uint8_t __resv2__ :2;
        
    };

    struct R8_AutoGain:public Reg8<>{
        static constexpr auto address = RegAddress::AutoGain;
        uint8_t data;
    };

    struct R16_Magnitude:public Reg16<>{
        static constexpr auto address = RegAddress::Magnitude;
        uint16_t data;
    };

    struct R8_Burn:public Reg8<>{
        static constexpr auto address = RegAddress::Burn;

        uint8_t data;
    };

    R8_ProgramTimes program_times_reg = {};
    R16_StartAngle start_angle_reg = {};
    R16_EndAngle end_angle_reg = {};
    R16_AmountAngle amount_angle_reg = {};
    R16_RawAngle raw_angle_reg = {};
    R16_Angle angle_reg = {};
    R16_Config config_reg = {};
    R8_Status status_reg = {};
    R8_AutoGain auto_gain_reg = {};
    R16_Magnitude magnitude_reg = {};
    R8_Burn burn_reg = {};
};

class AS5600 final: 
    public MagEncoderIntf,
    public AS5600_Regs{
public:
    AS5600(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AS5600(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}

    AS5600(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
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
            std::bit_cast<uint8_t>(reg.address), 
            reg.as_val(), LSB); 
            err.is_err()) return Err(err.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto err = i2c_drv_.read_reg(
            std::bit_cast<uint8_t>(reg.address), 
            reg.as_ref(), LSB); 
            err.is_err()) return Err(err.unwrap_err());
        return Ok();
    }

};


}
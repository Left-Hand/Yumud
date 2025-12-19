#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

namespace ymd::drivers{

struct AS5600_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x1e >> 1);

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

    enum class RegAddr : uint8_t {
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

struct AS5600_Regs:public AS5600_Prelude{

    struct R8_ProgramTimes:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::ProgramTimes;
        uint8_t times :2;
        uint8_t __resv__ :6;
        
    };

    struct R16_StartAngle:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::StartAngle;
        uint16_t bits;
    };

    struct R16_EndAngle:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::EndAngle;
        uint16_t bits;
    };

    struct R16_AmountAngle:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::AmountAngle;        
        uint16_t bits;
    };

    struct R16_RawAngle:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::RawAngle;
        uint16_t bits;
    };

    struct R16_Angle:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Angle;
        uint16_t bits;
    };

    struct R16_Config:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Config;
        uint8_t power_mode :2;
        uint8_t hysteresis:2;
        uint8_t output_stage:2;
        uint8_t pwm_frequency:2;
        uint8_t slow_filter:2;
        uint8_t fast_filter:3;
        uint8_t watch_dog:1;
        uint8_t __resv__ :2;
    };

    struct R8_Status:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Status;
        
        uint8_t __resv1__ :3;
        uint8_t mag_high:1;
        uint8_t mag_low:1;
        uint8_t mag_proper:1;
        uint8_t __resv2__ :2;
        
    };

    struct R8_AutoGain:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::AutoGain;
        uint8_t bits;
    };

    struct R16_Magnitude:public Reg16<>{
        static constexpr auto ADDRESS = RegAddr::Magnitude;
        uint16_t bits;
    };

    struct R8_Burn:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr::Burn;

        uint8_t bits;
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


}
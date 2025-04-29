#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

namespace ymd::drivers{

class AS5600 final: public MagEncoderIntf{
public:
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

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

protected:
    hal::I2cDrv i2c_drv_;

    struct ProgramTimesReg:public Reg8<>{
        
        uint8_t times :2;
        uint8_t __resv__ :6;
        
    };

    struct StartAngleReg:public Reg16<>{
        uint16_t data;
        
    };

    struct EndAngleReg:public Reg16<>{
        uint16_t data;
        
    };

    struct AmountAngleReg:public Reg16<>{
        uint16_t data;
        
    };

    struct RawAngleReg:public Reg16<>{
        uint16_t data;
    };

    struct AngleReg:public Reg16<>{
        uint16_t data;
    };

    struct ConfigReg:public Reg16<>{
        
        uint8_t powerMode :2;
        uint8_t hysteresis:2;
        uint8_t outputStage:2;
        uint8_t pwmFrequency:2;
        uint8_t slowFilter:2;
        uint8_t fastFilter:3;
        uint8_t watchDog:1;
        uint8_t __resv__ :2;
        
    };

    struct StatusReg:public Reg8<>{
        
        uint8_t __resv1__ :3;
        uint8_t magHigh:1;
        uint8_t magLow:1;
        uint8_t magProper:1;
        uint8_t __resv2__ :2;
        
    };

    struct AutoGainReg:public Reg8<>{
        uint8_t data;
    };

    struct MagnitudeReg:public Reg16<>{
        uint16_t data;
    };

    struct BurnReg:public Reg8<>{
        uint8_t data;
    };

    ProgramTimesReg programTimesReg = {};
    StartAngleReg startAngleReg = {};
    EndAngleReg endAngleReg = {};
    AmountAngleReg amountAngleReg = {};
    RawAngleReg rawAngleReg = {};
    AngleReg angleReg = {};
    ConfigReg configReg = {};
    StatusReg statusReg = {};
    AutoGainReg autoGainReg = {};
    MagnitudeReg magnitudeReg = {};
    BurnReg burnReg = {};

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

    real_t From12BitTo360Degrees(const uint16_t data){
        auto uni = u16_to_uni(data << 4);
        return uni * 360;
    }

    uint16_t From360DegreesTo12Bit(const real_t degrees){
        return uni_to_u16(CLAMP(degrees / 360, real_t(0), real_t(1))) >> 4;
    }

    [[nodiscard]]
    IResult<void> write_reg(const RegAddress addr, const uint16_t data){
        if(const auto err = i2c_drv_.write_reg(uint8_t(addr), data, LSB); err.is_err())
            return Err(Error::HalError(err.unwrap_err()));
        return Ok();
    }

    [[nodiscard]]
    IResult<void> read_reg(const RegAddress addr, uint16_t & data){
        if(const auto err = i2c_drv_.read_reg(uint8_t(addr), data, LSB); err.is_err())
            return Err(Error::HalError(err.unwrap_err()));
        return Ok();
    }

    [[nodiscard]]
    IResult<void> write_reg(const RegAddress addr, const uint8_t data){
        if(const auto err = i2c_drv_.write_reg(uint8_t(addr), data); err.is_err())
            return Err(Error::HalError(err.unwrap_err()));
        return Ok();
    }

    [[nodiscard]]
    IResult<void> read_reg(const RegAddress addr, uint8_t & data){
        if(const auto err = i2c_drv_.read_reg(uint8_t(addr), data); err.is_err())
            return Err(Error::HalError(err.unwrap_err()));
        return Ok();
    }
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x1e);

    AS5600(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AS5600(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}

    AS5600(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    [[nodiscard]]
    IResult<> set_power_mode(const PowerMode _power_mode);

    [[nodiscard]]
    IResult<> set_fast_filter(const FastFilter _fast_filter);

    [[nodiscard]]
    IResult<> set_slow_filter(const SlowFilter _slow_filter);

    [[nodiscard]]
    IResult<> set_pwm_frequency(const PwmFrequency _pwm_frequency);

    [[nodiscard]]
    IResult<> set_ouput_stage(const OutputStage _output_stage);

    [[nodiscard]]
    IResult<> set_hysteresis(const Hysteresis _hysteresis);
    
    [[nodiscard]]
    IResult<int8_t> get_mag_status();
    
    [[nodiscard]]
    IResult<uint8_t> get_gain();
    
    [[nodiscard]]
    IResult<uint16_t> get_magnitude();
    
    [[nodiscard]]
    IResult<real_t> get_raw_angle();
    
    [[nodiscard]]
    IResult<real_t> get_angle();
    
    [[nodiscard]]
    IResult<> set_start_angle(const real_t angle);
    
    [[nodiscard]]
    IResult<> set_end_angle(const real_t angle);
    
    [[nodiscard]]
    IResult<> set_amount_angle(const real_t angle);
    
    [[nodiscard]]
    IResult<uint8_t> get_program_times();
    
    [[nodiscard]]
    IResult<> burn_angle();
    
    [[nodiscard]]
    IResult<> burn_setting();
    
    [[nodiscard]]
    IResult<> init();
};


}
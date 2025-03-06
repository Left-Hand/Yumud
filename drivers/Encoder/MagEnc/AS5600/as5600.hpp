#pragma once
#include "drivers/device_defs.h"


namespace ymd::drivers{
class AS5600{
public:
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

    ProgramTimesReg programTimesReg;
    StartAngleReg startAngleReg;
    EndAngleReg endAngleReg;
    AmountAngleReg amountAngleReg;
    RawAngleReg rawAngleReg;
    AngleReg angleReg;
    ConfigReg configReg;
    StatusReg statusReg;
    AutoGainReg autoGainReg;
    MagnitudeReg magnitudeReg;
    BurnReg burnReg;

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

    void writeReg(const RegAddress addr, const uint16_t data){
        i2c_drv_.writeReg((uint8_t)addr, data, LSB);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        i2c_drv_.readReg((uint8_t)addr, data, LSB);
    }

    void writeReg(const RegAddress addr, const uint8_t data){
        i2c_drv_.writeReg((uint8_t)addr, data);
    }

    void readReg(const RegAddress addr, uint8_t & data){
        i2c_drv_.readReg((uint8_t)addr, data);
    }
public:
    scexpr uint8_t default_i2c_addr = 0x1e;

    AS5600(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AS5600(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}

    AS5600(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(hal::I2cDrv{i2c, i2c_addr}){;}

    void setPowerMode(const PowerMode _power_mode);

    void setFastFilter(const FastFilter _fast_filter);

    void setSlowFilter(const SlowFilter _slow_filter);

    void setPwmFrequency(const PwmFrequency _pwm_frequency);

    void setOuputStage(const OutputStage _output_stage);

    void setHysteresis(const Hysteresis _hysteresis);

    int8_t getMagStatus();

    uint8_t getGain();

    uint16_t getMagnitude();

    real_t getRawAngle();

    real_t getAngle();

    void setStartAngle(const real_t angle);

    void setEndAngle(const real_t angle);

    void setAmountAngle(const real_t angle);

    uint8_t getProgramTimes();

    void burnAngle();

    void burnSetting();

    void init();
};


}
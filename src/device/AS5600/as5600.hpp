#ifndef __AS5600_HPP__

#define __AS5600_HPP__

#include "device_defs.h"
#include "types/real.hpp"

#ifndef REG8_BEGIN
#define REG8_BEGIN union{struct{
#endif

#ifndef REG8_END
#define REG8_END };uint8_t data;};
#endif

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
    I2cDrv & bus_drv;

    struct ProgramTimesReg:public Reg8{
        REG8_BEGIN
        uint8_t times :2;
        uint8_t __resv__ :6;
        REG8_END
    };

    struct StartAngleReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct EndAngleReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct AmountAngleReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct RawAngleReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct AngleReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct ConfigReg:public Reg16{
        REG16_BEGIN
        uint8_t powerMode :2;
        uint8_t hysteresis:2;
        uint8_t outputStage:2;
        uint8_t pwmFrequency:2;
        uint8_t slowFilter:2;
        uint8_t fastFilter:3;
        uint8_t watchDog:1;
        uint8_t __resv__ :2;
        REG16_END
    };

    struct StatusReg:public Reg8{
        REG8_BEGIN
        uint8_t __resv1__ :1;
        uint8_t magHigh:1;
        uint8_t magLow:1;
        uint8_t magProper:1;
        REG8_END
    };

    struct AutoGainReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct MagnitudeReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct BurnReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct{
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

    real_t From12BitTo360Degrees(const uint16_t & data){
        real_t uni;
        u16_to_uni(data, uni);
        return uni * 360;
    }

    uint16_t From360DegreesTo12Bit(const real_t degrees){
        uint16_t ret;
        uni_to_u16(CLAMP(degrees / real_t(360), real_t(0), real_t(1)), ret);
        return ret;
    }

    void writeReg(const RegAddress & regAddress, const Reg16 & regData){
        bus_drv.writeReg((uint8_t)regAddress, *(uint16_t *)&regData);
    }

    void readReg(const RegAddress & regAddress, Reg16 & regData){
        uint16_t temp = 0;
        bus_drv.readReg((uint8_t)regAddress, temp);
        regData = *(Reg16 *)&temp;
    }

    void writeReg(const RegAddress & regAddress, const Reg8 & regData){
        bus_drv.writeReg((uint8_t)regAddress, *(uint8_t *)&regData);
    }

    void readReg(const RegAddress & regAddress, Reg8 & regData){
        bus_drv.readReg((uint8_t)regAddress, *(uint8_t *)&regData);
    }

    void requestRegData(const RegAddress & regAddress, uint8_t * data_ptr, const size_t len){
        bus_drv.readPool((uint8_t)regAddress, data_ptr, 2, len, false);
    }

public:
    AS5600(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}

    void setPowerMode(const PowerMode & _power_mode){
        configReg.powerMode = (uint8_t)_power_mode;
        writeReg(RegAddress::Config, configReg);
    }

    void setFastFilter(const FastFilter & _fast_filter){
        configReg.fastFilter = (uint8_t)_fast_filter;
        writeReg(RegAddress::Config, configReg);
    }

    void setSlowFilter(const SlowFilter & _slow_filter){
        configReg.slowFilter = (uint8_t)_slow_filter;
        writeReg(RegAddress::Config, configReg);
    }

    void setPwmFrequency(const PwmFrequency & _pwm_frequency){
        configReg.pwmFrequency = (uint8_t)_pwm_frequency;
        writeReg(RegAddress::Config, configReg);
    }

    void setOuputStage(const OutputStage & _output_stage){
        configReg.outputStage = (uint8_t)_output_stage;
        writeReg(RegAddress::Config, configReg);
    }

    void setHysteresis(const Hysteresis & _hysteresis){
        configReg.hysteresis = (uint8_t)_hysteresis;
        writeReg(RegAddress::Config, configReg);
    }

    int8_t getMagStatus(){
        readReg(RegAddress::Status, statusReg);
        if(statusReg.magProper) return 0;
        else if(statusReg.magHigh) return 1;
        else if(statusReg.magLow) return -1;
    }

    uint8_t getGain(){
        readReg(RegAddress::AutoGain, autoGainReg);
        return autoGainReg.data;
    }

    uint16_t getMagnitude(){
        readReg(RegAddress::Magnitude, magnitudeReg);
        return (magnitudeReg.data) & 0xFFF;
    }

    real_t getRawAngle(){
        readReg(RegAddress::RawAngle, rawAngleReg);
        return From12BitTo360Degrees(rawAngleReg.data);
    }

    real_t getAngle(){
        readReg(RegAddress::Angle, angleReg);
        return From12BitTo360Degrees(angleReg.data);
    }

    void setStartAngle(const real_t & angle){
        startAngleReg.data = From360DegreesTo12Bit(angle);
        writeReg(RegAddress::StartAngle, startAngleReg);
    }

    void setEndAngle(const real_t & angle){
        endAngleReg.data = From360DegreesTo12Bit(angle);
        writeReg(RegAddress::EndAngle, endAngleReg);
    } 

    void setAmountAngle(const real_t & angle){
        amountAngleReg.data = From360DegreesTo12Bit(angle);
        writeReg(RegAddress::AmountAngle, amountAngleReg);
    }

    uint8_t getProgramTimes(){
        readReg(RegAddress::ProgramTimes, programTimesReg);
        return programTimesReg.times;
    }

    void burnAngle(){
        burnReg.data = 0x80;
        writeReg(RegAddress::Burn, burnReg);
    }

    void burnSetting(){
        burnReg.data = 0x80;
        writeReg(RegAddress::Burn, burnReg);
    }

    void init(){
        setPowerMode(PowerMode::Norm);
    }
};

#endif
#ifndef __AS5600_HPP__

#define __AS5600_HPP__

#include "../bus/busdrv.hpp"

#ifndef REG16_BEGIN
#define REG16_BEGIN union{struct{
#endif

#ifndef REG16_END
#define REG16_END };uint16_t data;};
#endif

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
    BusDrv & busdrv;

    struct Reg16{};

    struct Reg8{};

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

    float From12BitTo360Degrees(const uint16_t & data){
        return data * 360.0f / 0xFFF; 
    }

    uint16_t From360DegreesTo12Bit(const float degrees){
        return CLAMP(degrees, 0, 360) * 0xFFF / 360.0f;
    }

    void writeReg(const RegAddress & regAddress, const Reg16 & regData){
        busdrv.writeReg((uint8_t)regAddress, *(uint16_t *)&regData);
    }

    void readReg(const RegAddress & regAddress, Reg16 & regData){
        uint16_t temp = 0;
        busdrv.readReg((uint8_t)regAddress, temp);
        regData = *(Reg16 *)&temp;
    }

    void writeReg(const RegAddress & regAddress, const Reg8 & regData){
        busdrv.writeReg((uint8_t)regAddress, *(uint8_t *)&regData);
    }

    void readReg(const RegAddress & regAddress, Reg8 & regData){
        busdrv.readReg((uint8_t)regAddress, *(uint8_t *)&regData);
    }

    void requestRegData(const RegAddress & regAddress, uint8_t * data_ptr, const size_t len){
        busdrv.readPool((uint8_t)regAddress, data_ptr, 2, len, false);
    }

public:
    AS5600(BusDrv & _busdrv):busdrv(_busdrv){;}

    void setPowerMode(const PowerMode & _powermode){
        configReg.powerMode = (uint8_t)_powermode;
        writeReg(RegAddress::Config, configReg);
    }

    void setFastFilter(const FastFilter & _fastfilter){
        configReg.fastFilter = (uint8_t)_fastfilter;
        writeReg(RegAddress::Config, configReg);
    }

    void setSlowFilter(const SlowFilter & _slowfilter){
        configReg.slowFilter = (uint8_t)_slowfilter;
        writeReg(RegAddress::Config, configReg);
    }

    void setPwmFrequency(const PwmFrequency & _pwmfrequency){
        configReg.pwmFrequency = (uint8_t)_pwmfrequency;
        writeReg(RegAddress::Config, configReg);
    }

    void setOuputStage(const OutputStage & _outputstage){
        configReg.outputStage = (uint8_t)_outputstage;
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

    float getRawAngle(){
        readReg(RegAddress::RawAngle, rawAngleReg);
        return From12BitTo360Degrees(rawAngleReg.data);
    }

    float getAngle(){
        readReg(RegAddress::Angle, angleReg);
        return From12BitTo360Degrees(angleReg.data);
    }

    void setStartAngle(const float & angle){
        startAngleReg.data = From360DegreesTo12Bit(angle);
        writeReg(RegAddress::StartAngle, startAngleReg);
    }

    void setEndAngle(const float & angle){
        endAngleReg.data = From360DegreesTo12Bit(angle);
        writeReg(RegAddress::EndAngle, endAngleReg);
    } 

    void setAmountAngle(const float & angle){
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
        ;
    }
};

#endif
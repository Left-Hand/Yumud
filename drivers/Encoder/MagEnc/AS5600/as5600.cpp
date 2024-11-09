#include "as5600.hpp"

using namespace yumud::drivers;


void AS5600::setPowerMode(const PowerMode _power_mode){
    configReg.powerMode = (uint8_t)_power_mode;
    writeReg(RegAddress::Config, configReg);
}

void AS5600::setFastFilter(const FastFilter _fast_filter){
    configReg.fastFilter = (uint8_t)_fast_filter;
    writeReg(RegAddress::Config, configReg);
}

void AS5600::setSlowFilter(const SlowFilter _slow_filter){
    configReg.slowFilter = (uint8_t)_slow_filter;
    writeReg(RegAddress::Config, configReg);
}

void AS5600::setPwmFrequency(const PwmFrequency _pwm_frequency){
    configReg.pwmFrequency = (uint8_t)_pwm_frequency;
    writeReg(RegAddress::Config, configReg);
}

void AS5600::setOuputStage(const OutputStage _output_stage){
    configReg.outputStage = (uint8_t)_output_stage;
    writeReg(RegAddress::Config, configReg);
}

void AS5600::setHysteresis(const Hysteresis _hysteresis){
    configReg.hysteresis = (uint8_t)_hysteresis;
    writeReg(RegAddress::Config, configReg);
}

int8_t AS5600::getMagStatus(){
    readReg(RegAddress::Status, statusReg);
    if(statusReg.magProper) return 0;
    else if(statusReg.magHigh) return 1;
    else return -1;
}

uint8_t AS5600::getGain(){
    readReg(RegAddress::AutoGain, autoGainReg.data);
    return autoGainReg.data;
}

uint16_t AS5600::getMagnitude(){
    readReg(RegAddress::Magnitude, magnitudeReg.data);
    return (magnitudeReg.data) & 0xFFF;
}

real_t AS5600::getRawAngle(){
    readReg(RegAddress::RawAngle, rawAngleReg.data);
    return From12BitTo360Degrees(rawAngleReg.data);
    // return (real_t)(int)rawAngleReg.data;
}

real_t AS5600::getAngle(){
    readReg(RegAddress::Angle, angleReg.data);
    return From12BitTo360Degrees(angleReg.data);
}

void AS5600::setStartAngle(const real_t angle){
    startAngleReg.data = From360DegreesTo12Bit(angle);
    writeReg(RegAddress::StartAngle, startAngleReg.data);
}

void AS5600::setEndAngle(const real_t angle){
    endAngleReg.data = From360DegreesTo12Bit(angle);
    writeReg(RegAddress::EndAngle, endAngleReg.data);
}

void AS5600::setAmountAngle(const real_t angle){
    amountAngleReg.data = From360DegreesTo12Bit(angle);
    writeReg(RegAddress::AmountAngle, amountAngleReg.data);
}

uint8_t AS5600::getProgramTimes(){
    readReg(RegAddress::ProgramTimes, programTimesReg);
    return programTimesReg.times;
}

void AS5600::burnAngle(){
    burnReg.data = 0x80;
    writeReg(RegAddress::Burn, burnReg.data);
}

void AS5600::burnSetting(){
    burnReg.data = 0x80;
    writeReg(RegAddress::Burn, burnReg.data);
}

void AS5600::init(){
    setPowerMode(PowerMode::Norm);
}
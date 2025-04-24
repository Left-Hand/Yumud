#include "as5600.hpp"

using namespace ymd::drivers;


void AS5600::set_power_mode(const PowerMode _power_mode){
    configReg.powerMode = (uint8_t)_power_mode;
    write_reg(RegAddress::Config, configReg);
}

void AS5600::set_fast_filter(const FastFilter _fast_filter){
    configReg.fastFilter = (uint8_t)_fast_filter;
    write_reg(RegAddress::Config, configReg);
}

void AS5600::set_slow_filter(const SlowFilter _slow_filter){
    configReg.slowFilter = (uint8_t)_slow_filter;
    write_reg(RegAddress::Config, configReg);
}

void AS5600::set_pwm_frequency(const PwmFrequency _pwm_frequency){
    configReg.pwmFrequency = (uint8_t)_pwm_frequency;
    write_reg(RegAddress::Config, configReg);
}

void AS5600::set_ouput_stage(const OutputStage _output_stage){
    configReg.outputStage = (uint8_t)_output_stage;
    write_reg(RegAddress::Config, configReg);
}

void AS5600::set_hysteresis(const Hysteresis _hysteresis){
    configReg.hysteresis = (uint8_t)_hysteresis;
    write_reg(RegAddress::Config, configReg);
}

int8_t AS5600::get_mag_status(){
    read_reg(RegAddress::Status, statusReg);
    if(statusReg.magProper) return 0;
    else if(statusReg.magHigh) return 1;
    else return -1;
}

uint8_t AS5600::get_gain(){
    read_reg(RegAddress::AutoGain, autoGainReg.data);
    return autoGainReg.data;
}

uint16_t AS5600::get_magnitude(){
    read_reg(RegAddress::Magnitude, magnitudeReg.data);
    return (magnitudeReg.data) & 0xFFF;
}

real_t AS5600::get_raw_angle(){
    read_reg(RegAddress::RawAngle, rawAngleReg.data);
    return From12BitTo360Degrees(rawAngleReg.data);
    // return (real_t)(int)rawAngleReg.data;
}

real_t AS5600::get_angle(){
    read_reg(RegAddress::Angle, angleReg.data);
    return From12BitTo360Degrees(angleReg.data);
}

void AS5600::set_start_angle(const real_t angle){
    startAngleReg.data = From360DegreesTo12Bit(angle);
    write_reg(RegAddress::StartAngle, startAngleReg.data);
}

void AS5600::set_end_angle(const real_t angle){
    endAngleReg.data = From360DegreesTo12Bit(angle);
    write_reg(RegAddress::EndAngle, endAngleReg.data);
}

void AS5600::set_amount_angle(const real_t angle){
    amountAngleReg.data = From360DegreesTo12Bit(angle);
    write_reg(RegAddress::AmountAngle, amountAngleReg.data);
}

uint8_t AS5600::get_program_times(){
    read_reg(RegAddress::ProgramTimes, programTimesReg);
    return programTimesReg.times;
}

void AS5600::burn_angle(){
    burnReg.data = 0x80;
    write_reg(RegAddress::Burn, burnReg.data);
}

void AS5600::burn_setting(){
    burnReg.data = 0x80;
    write_reg(RegAddress::Burn, burnReg.data);
}

void AS5600::init(){
    set_power_mode(PowerMode::Norm);
}
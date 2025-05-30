#include "as5600.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = AS5600::Error;

template<typename T = void>
using IResult = AS5600::IResult<T>;


static constexpr real_t From12BitTo360Degrees(const uint16_t data){
    auto uni = u16_to_uni(data << 4);
    return uni * 360;
}

static constexpr uint16_t From360DegreesTo12Bit(const real_t degrees){
    return uni_to_u16(CLAMP(degrees / 360, real_t(0), real_t(1))) >> 4;
}
IResult<> AS5600::set_power_mode(const PowerMode _power_mode){
    configReg.powerMode = uint8_t(_power_mode);
    return write_reg(RegAddress::Config, configReg);
}

IResult<> AS5600::set_fast_filter(const FastFilter _fast_filter){
    configReg.fastFilter = uint8_t(_fast_filter);
    return write_reg(RegAddress::Config, configReg);
}

IResult<> AS5600::set_slow_filter(const SlowFilter _slow_filter){
    configReg.slowFilter = uint8_t(_slow_filter);
    return write_reg(RegAddress::Config, configReg);
}

IResult<> AS5600::set_pwm_frequency(const PwmFrequency _pwm_frequency){
    configReg.pwmFrequency = uint8_t(_pwm_frequency);
    return write_reg(RegAddress::Config, configReg);
}

IResult<> AS5600::set_ouput_stage(const OutputStage _output_stage){
    configReg.outputStage = uint8_t(_output_stage);
    return write_reg(RegAddress::Config, configReg);
}

IResult<> AS5600::set_hysteresis(const Hysteresis _hysteresis){
    configReg.hysteresis = uint8_t(_hysteresis);
    return write_reg(RegAddress::Config, configReg);
}

IResult<MagStatus> AS5600::get_mag_status(){
    // UNWRAP_OR_RETURN_ERR(read_reg(RegAddress::Status, statusReg));
    if(const auto res = read_reg(RegAddress::Status, statusReg);
        res.is_err()) return Err(res.unwrap_err());
    if(statusReg.magProper) return Ok(MagStatus::Proper());
    else if(statusReg.magHigh) return Ok(MagStatus::High());
    else if(statusReg.magLow) return Ok(MagStatus::Low());
    else return Ok(MagStatus(MagStatus::Invalid()));
}

IResult<uint8_t> AS5600::get_gain(){
    if(const auto res = read_reg(RegAddress::AutoGain, autoGainReg.data); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(autoGainReg.data);
}

IResult<uint16_t> AS5600::get_magnitude(){
    if(const auto res = read_reg(RegAddress::Magnitude, magnitudeReg.data); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok((magnitudeReg.data) & 0xFFF);
}

IResult<real_t> AS5600::get_raw_angle(){
    if(const auto res = read_reg(RegAddress::RawAngle, rawAngleReg.data);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(From12BitTo360Degrees(rawAngleReg.data));
}

IResult<real_t> AS5600::get_angle(){
    if(const auto res = read_reg(RegAddress::Angle, angleReg.data);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(From12BitTo360Degrees(angleReg.data));
}

IResult<> AS5600::set_start_angle(const real_t angle){
    startAngleReg.data = From360DegreesTo12Bit(angle);
    return write_reg(RegAddress::StartAngle, startAngleReg.data);
}

IResult<> AS5600::set_end_angle(const real_t angle){
    endAngleReg.data = From360DegreesTo12Bit(angle);
    return write_reg(RegAddress::EndAngle, endAngleReg.data);
}

IResult<> AS5600::set_amount_angle(const real_t angle){
    amountAngleReg.data = From360DegreesTo12Bit(angle);
    return write_reg(RegAddress::AmountAngle, amountAngleReg.data);
}

IResult<uint8_t> AS5600::get_program_times(){
    if(const auto res = read_reg(RegAddress::ProgramTimes, programTimesReg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(uint8_t(programTimesReg.times));
}

IResult<> AS5600::burn_angle(){
    burnReg.data = 0x80;
    return write_reg(RegAddress::Burn, burnReg.data);
}

IResult<> AS5600::burn_setting(){
    burnReg.data = 0x80;
    return write_reg(RegAddress::Burn, burnReg.data);
}

IResult<> AS5600::init(){
    return set_power_mode(PowerMode::Norm);
}
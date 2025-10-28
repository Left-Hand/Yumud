#include "as5600.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = AS5600::Error;

template<typename T = void>
using IResult = AS5600::IResult<T>;


static constexpr Angle<q31> u12_to_angle(const uint16_t data){
    return Angle<q31>::from_turns(q31(q16::from_i32(data << 4)));
}

static constexpr uint16_t angle_to_u12(const Angle<q31> angle){
    return q16(angle.to_turns()).as_i32() >> 4; 
}

IResult<> AS5600::set_power_mode(const PowerMode power_mode){
    auto reg = RegCopy(config_reg);
    reg.powerMode = uint8_t(power_mode);
    return write_reg(reg);
}

IResult<> AS5600::set_fast_filter(const FastFilter fast_filter){
    auto reg = RegCopy(config_reg);
    reg.fastFilter = uint8_t(fast_filter);
    return write_reg(reg);
}

IResult<> AS5600::set_slow_filter(const SlowFilter slow_filter){
    auto reg = RegCopy(config_reg);
    reg.slowFilter = uint8_t(slow_filter);
    return write_reg(reg);
}

IResult<> AS5600::set_pwm_frequency(const PwmFrequency pwm_frequency){
    auto reg = RegCopy(config_reg);
    reg.pwmFrequency = uint8_t(pwm_frequency);
    return write_reg(reg);
}

IResult<> AS5600::set_ouput_stage(const OutputStage output_stage){
    auto reg = RegCopy(config_reg);
    reg.outputStage = uint8_t(output_stage);
    return write_reg(reg);
}

IResult<> AS5600::set_hysteresis(const Hysteresis hysteresis){
    auto reg = RegCopy(config_reg);
    reg.hysteresis = uint8_t(hysteresis);
    return write_reg(reg);
}

IResult<MagStatus> AS5600::get_mag_status(){
    if(const auto res = read_reg(status_reg);
        res.is_err()) return Err(res.unwrap_err());
    if(status_reg.magProper) return Ok(MagStatus::from_proper());
    else if(status_reg.magHigh) return Ok(MagStatus::from_high());
    else if(status_reg.magLow) return Ok(MagStatus::from_low());
    else return Ok(MagStatus(MagStatus::from_invalid()));
}

IResult<uint8_t> AS5600::get_gain(){
    if(const auto res = read_reg(auto_gain_reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(auto_gain_reg.data);
}

IResult<uint16_t> AS5600::get_magnitude(){
    if(const auto res = read_reg(magnitude_reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok((magnitude_reg.data) & 0xFFF);
}

IResult<Angle<q31>> AS5600::get_raw_angle(){
    if(const auto res = read_reg(raw_angle_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(u12_to_angle(raw_angle_reg.data));
}

IResult<Angle<q31>> AS5600::get_angle(){
    if(const auto res = read_reg(angle_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(u12_to_angle(angle_reg.data));
}

IResult<> AS5600::set_start_angle(const Angle<q31> angle){
    auto reg = RegCopy(start_angle_reg);
    reg.data = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<> AS5600::set_end_angle(const Angle<q31> angle){
    auto reg = RegCopy(end_angle_reg);
    reg.data = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<> AS5600::set_amount_angle(const Angle<q31> angle){
    auto reg = RegCopy(amount_angle_reg);
    reg.data = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<uint8_t> AS5600::get_program_times(){
    if(const auto res = read_reg(program_times_reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(uint8_t(program_times_reg.times));
}

IResult<> AS5600::burn_angle(){
    auto reg = RegCopy(burn_reg);
    reg.data = 0x80;
    return write_reg(reg);
}

IResult<> AS5600::burn_setting(){
    auto reg = RegCopy(burn_reg);
    reg.data = 0x80;
    return write_reg(reg);
}

IResult<> AS5600::init(){
    return set_power_mode(PowerMode::Norm);
}
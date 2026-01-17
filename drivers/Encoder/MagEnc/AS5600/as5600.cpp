#include "as5600.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Self = AS5600;

using Error = Self::Error;

template<typename T = void>
using IResult = Self::IResult<T>;


static constexpr Angular<uq32> u12_to_angle(const uint16_t bits){
    return Angular<uq32>::from_turns(uq32(uq16::from_bits(bits << 4)));
}

static constexpr uint16_t angle_to_u12(const Angular<uq32> angle){
    return uq16(angle.to_turns()).to_bits() >> 4; 
}

IResult<> Self::set_power_mode(const PowerMode power_mode){
    auto reg = RegCopy(config_reg);
    reg.power_mode = power_mode;
    return write_reg(reg);
}

IResult<> Self::set_fast_filter(const FastFilter fast_filter){
    auto reg = RegCopy(config_reg);
    reg.fast_filter = fast_filter;
    return write_reg(reg);
}

IResult<> Self::set_slow_filter(const SlowFilter slow_filter){
    auto reg = RegCopy(config_reg);
    reg.slow_filter = slow_filter;
    return write_reg(reg);
}

IResult<> Self::set_pwm_frequency(const PwmFrequency pwm_frequency){
    auto reg = RegCopy(config_reg);
    reg.pwm_frequency = pwm_frequency;
    return write_reg(reg);
}

IResult<> Self::set_ouput_stage(const OutputStage output_stage){
    auto reg = RegCopy(config_reg);
    reg.output_stage = output_stage;
    return write_reg(reg);
}

IResult<> Self::set_hysteresis(const Hysteresis hysteresis){
    auto reg = RegCopy(config_reg);
    reg.hysteresis = hysteresis;
    return write_reg(reg);
}

IResult<MagStatus> Self::get_mag_status(){
    if(const auto res = read_reg(status_reg);
        res.is_err()) return Err(res.unwrap_err());
    if(status_reg.mag_proper) return Ok(MagStatus::from_proper());
    else if(status_reg.mag_high) return Ok(MagStatus::from_high());
    else if(status_reg.mag_low) return Ok(MagStatus::from_low());
    else return Ok(MagStatus(MagStatus::from_invalid()));
}

IResult<uint8_t> Self::get_gain(){
    if(const auto res = read_reg(auto_gain_reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(auto_gain_reg.bits);
}

IResult<uint16_t> Self::get_magnitude(){
    if(const auto res = read_reg(magnitude_reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok((magnitude_reg.bits) & 0xFFF);
}

IResult<Angular<uq32>> Self::get_raw_angle(){
    if(const auto res = read_reg(raw_angle_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(u12_to_angle(raw_angle_reg.bits));
}

IResult<Angular<uq32>> Self::get_angle(){
    if(const auto res = read_reg(angle_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(u12_to_angle(angle_reg.bits));
}

IResult<> Self::set_start_angle(const Angular<uq32> angle){
    auto reg = RegCopy(start_angle_reg);
    reg.bits = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<> Self::set_end_angle(const Angular<uq32> angle){
    auto reg = RegCopy(end_angle_reg);
    reg.bits = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<> Self::set_amount_angle(const Angular<uq32> angle){
    auto reg = RegCopy(amount_angle_reg);
    reg.bits = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<uint8_t> Self::get_program_times(){
    if(const auto res = read_reg(program_times_reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(uint8_t(program_times_reg.times));
}

IResult<> Self::burn_angle(){
    auto reg = RegCopy(burn_reg);
    reg.bits = 0x80;
    return write_reg(reg);
}

IResult<> Self::burn_setting(){
    auto reg = RegCopy(burn_reg);
    reg.bits = 0x80;
    return write_reg(reg);
}

IResult<> Self::init(){
    return set_power_mode(PowerMode::Norm);
}
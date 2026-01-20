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
    auto reg = RegCopy(regs_.config_reg);
    reg.power_mode = power_mode;
    return write_reg(reg);
}

IResult<> Self::set_fast_filter(const FastFilter fast_filter){
    auto reg = RegCopy(regs_.config_reg);
    reg.fast_filter = fast_filter;
    return write_reg(reg);
}

IResult<> Self::set_slow_filter(const SlowFilter slow_filter){
    auto reg = RegCopy(regs_.config_reg);
    reg.slow_filter = slow_filter;
    return write_reg(reg);
}

IResult<> Self::set_pwm_frequency(const PwmFrequency pwm_frequency){
    auto reg = RegCopy(regs_.config_reg);
    reg.pwm_frequency = pwm_frequency;
    return write_reg(reg);
}

IResult<> Self::set_ouput_stage(const OutputStage output_stage){
    auto reg = RegCopy(regs_.config_reg);
    reg.output_stage = output_stage;
    return write_reg(reg);
}

IResult<> Self::set_hysteresis(const Hysteresis hysteresis){
    auto reg = RegCopy(regs_.config_reg);
    reg.hysteresis = hysteresis;
    return write_reg(reg);
}

IResult<EncoderFaultBitFields> Self::get_fault(){
    auto & reg = regs_.status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    EncoderFaultBitFields fault = EncoderFaultBitFields::zero();
    if(reg.mag_proper){
        fault.mag_strength = EncoderFaultBitFields::MagStrength::Proper;
    }else if(reg.mag_high){
        fault.mag_strength = EncoderFaultBitFields::MagStrength::High;
    } else if(reg.mag_low){
        fault.mag_strength = EncoderFaultBitFields::MagStrength::Low;
    }
    return Ok(fault);
}

IResult<uint8_t> Self::get_gain(){
    auto & reg = regs_.auto_gain_reg;
    if(const auto res = read_reg(reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.bits);
}

IResult<uint16_t> Self::get_magnitude(){
    auto & reg = regs_.magnitude_reg;
    if(const auto res = read_reg(reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(static_cast<uint16_t>((reg.bits) & 0xFFF));
}

IResult<Angular<uq32>> Self::get_raw_angle(){
    auto & reg = regs_.raw_angle_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(u12_to_angle(reg.bits));
}

IResult<Angular<uq32>> Self::get_angle(){
    auto & reg = regs_.angle_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(u12_to_angle(reg.bits));
}

IResult<> Self::set_start_angle(const Angular<uq32> angle){
    auto reg = RegCopy(regs_.start_angle_reg);
    reg.bits = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<> Self::set_end_angle(const Angular<uq32> angle){
    auto reg = RegCopy(regs_.end_angle_reg);
    reg.bits = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<> Self::set_amount_angle(const Angular<uq32> angle){
    auto reg = RegCopy(regs_.amount_angle_reg);
    reg.bits = angle_to_u12(angle);
    return write_reg(reg);
}

IResult<uint8_t> Self::get_program_times(){
    auto & reg = regs_.program_times_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(uint8_t(reg.times));
}

IResult<> Self::burn_angle(){
    auto reg = RegCopy(regs_.burn_reg);
    reg.bits = 0x80;
    return write_reg(reg);
}

IResult<> Self::burn_setting(){
    auto reg = RegCopy(regs_.burn_reg);
    reg.bits = 0x80;
    return write_reg(reg);
}

IResult<> Self::init(){
    return set_power_mode(PowerMode::Norm);
}
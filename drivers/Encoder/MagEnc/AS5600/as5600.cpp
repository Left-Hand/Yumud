#include "as5600.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = AS5600::Error;

template<typename T = void>
using IResult = AS5600::IResult<T>;


static constexpr q16 _12bits_2_pos(const uint16_t data){
    return q16::from_i32(data << 4);
}

static constexpr uint16_t pos_2_12bits(const q16 pos){
    return pos.to_i32() >> 4; 
}

IResult<> AS5600::set_power_mode(const PowerMode _power_mode){
    auto reg = RegCopy(config_reg);
    reg.powerMode = uint8_t(_power_mode);
    return write_reg(reg);
}

IResult<> AS5600::set_fast_filter(const FastFilter _fast_filter){
    auto reg = RegCopy(config_reg);
    reg.fastFilter = uint8_t(_fast_filter);
    return write_reg(reg);
}

IResult<> AS5600::set_slow_filter(const SlowFilter _slow_filter){
    auto reg = RegCopy(config_reg);
    reg.slowFilter = uint8_t(_slow_filter);
    return write_reg(reg);
}

IResult<> AS5600::set_pwm_frequency(const PwmFrequency _pwm_frequency){
    auto reg = RegCopy(config_reg);
    reg.pwmFrequency = uint8_t(_pwm_frequency);
    return write_reg(reg);
}

IResult<> AS5600::set_ouput_stage(const OutputStage _output_stage){
    auto reg = RegCopy(config_reg);
    reg.outputStage = uint8_t(_output_stage);
    return write_reg(reg);
}

IResult<> AS5600::set_hysteresis(const Hysteresis _hysteresis){
    auto reg = RegCopy(config_reg);
    reg.hysteresis = uint8_t(_hysteresis);
    return write_reg(reg);
}

IResult<MagStatus> AS5600::get_mag_status(){
    if(const auto res = read_reg(status_reg);
        res.is_err()) return Err(res.unwrap_err());
    if(status_reg.magProper) return Ok(MagStatus::Proper());
    else if(status_reg.magHigh) return Ok(MagStatus::High());
    else if(status_reg.magLow) return Ok(MagStatus::Low());
    else return Ok(MagStatus(MagStatus::Invalid()));
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

IResult<real_t> AS5600::get_raw_position(){
    if(const auto res = read_reg(raw_angle_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(_12bits_2_pos(raw_angle_reg.data));
}

IResult<real_t> AS5600::get_position(){
    if(const auto res = read_reg(angle_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(_12bits_2_pos(angle_reg.data));
}

IResult<> AS5600::set_start_position(const real_t angle){
    auto reg = RegCopy(start_angle_reg);
    reg.data = pos_2_12bits(angle);
    return write_reg(reg);
}

IResult<> AS5600::set_end_position(const real_t angle){
    auto reg = RegCopy(end_angle_reg);
    reg.data = pos_2_12bits(angle);
    return write_reg(reg);
}

IResult<> AS5600::set_amount_position(const real_t angle){
    auto reg = RegCopy(amount_angle_reg);
    reg.data = pos_2_12bits(angle);
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
#include "mt6701.hpp"
#include "core/debug/debug.hpp"



#ifdef MT6701_DEBUG
#undef MT6701_DEBUG
#define MT6701_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__)
#else
#define MT6701_DEBUG(...)
#endif


using namespace ymd::drivers;
using namespace ymd;

using Error = MT6701::Error;

template<typename T = void>
using IResult = Result<T, Error>;


static constexpr uint16_t perunit_angle_to_u12(const Angular<uq32> angle){
    return static_cast<uint16_t>(static_cast<uint32_t>(angle.to_turns().to_bits()) >> (32 - 12) & 0xfff);
}

IResult<> MT6701::init(){
    if(const auto res = enable_pwm(EN);
        res.is_err()) return res;
    if(const auto res = set_pwm_polarity(true);
        res.is_err()) return res;
    if(const auto res = set_pwm_freq(PwmFreq::HZ497_2);
        res.is_err()) return res;
    if(const auto res = update();
        res.is_err()) return res;

    return Ok();
}

IResult<> MT6701::update(){
    auto & reg = regs_.raw_angle_reg;
    const auto res = read_reg(reg);
    lap_position_ = uq32::from_bits(reg.bits);
    return res;
    // else if(spi_drv){

    //     uint16_t data16;
    //     spi_drv->read_single(data16).unwrap();

    //     uint8_t data8 = 0;
    //     if(fast_mode == false){
    //         spi_drv->read_single(data8).unwrap();
    //     }

    //     semantic = Semantic{data8, data16};
    //     if(semantic.valid(fast_mode)){
    //         lap_position = real_t(fixed_t<16>(semantic.data_14bit << 2) >> 16);
    //     } 
    // }
    // else{
    //     MT6701_DEBUG("no drv!!");
    //     PANIC();
    // }
}

IResult<Angular<uq32>> MT6701::read_lap_angle(){
    return Ok(Angular<uq32>::from_turns(lap_position_));
}


IResult<> MT6701::enable_uvwmux(const Enable en){
    auto reg = RegCopy(regs_.uvw_mux_reg);
    reg.uvw_mux = en == EN;
    return write_reg(reg);
}

IResult<> MT6701::enable_abzmux(const Enable en){
    auto reg = RegCopy(regs_.abz_mux_reg);
    reg.abz_mux = en == EN;
    return write_reg(reg);
}

IResult<> MT6701::set_direction(const RotateDirection dir){
    auto reg = RegCopy(regs_.abz_mux_reg);
    reg.is_clockwise = dir == RotateDirection::CW;
    return write_reg(reg);
}

IResult<> MT6701::set_pole_pairs(const uint8_t pole_pairs){
    auto reg = RegCopy(regs_.resolution_reg);
    reg.pole_pairs = pole_pairs;
    return write_reg(reg);
}

IResult<> MT6701::set_abz_resolution(const uint16_t abz_resolution){

    auto reg = RegCopy(regs_.resolution_reg);
    reg.abz_resolution = abz_resolution;
    return write_reg(reg);
}

IResult<> MT6701::set_zero_angle(
    const Angular<uq32> angle
){
    auto reg = RegCopy(regs_.zero_config_reg);
    reg.zero_position = perunit_angle_to_u12(angle);
    return write_reg(reg);
}

IResult<> MT6701::set_zero_pulse_width(
        const ZeroPulseWidth zero_pulse_width){

    auto reg = RegCopy(regs_.zero_config_reg);
    reg.zero_pulse_width = zero_pulse_width;
    return write_reg(reg);
}

IResult<> MT6701::set_hysteresis(const Hysteresis hysteresis){
    {
        auto reg = RegCopy(regs_.hystersis_reg);
        reg.hysteresis = static_cast<uint8_t>(hysteresis) & 0b11;
        if(const auto res = write_reg(reg);
            res.is_err()) return res;
    }
    {
        auto reg = RegCopy(regs_.zero_config_reg);
        reg.hysteresis = static_cast<uint8_t>(hysteresis) >> 2;
        return write_reg(reg);
    }
}

IResult<> MT6701::enable_fast_mode(const Enable en){
    fast_mode_ = en == EN;
    return Ok();
}

IResult<> MT6701::enable_pwm(const Enable en){
    auto reg = RegCopy(regs_.wire_config_reg);
    reg.pwm_en = en == EN;
    return write_reg(reg);
}

IResult<> MT6701::set_pwm_polarity(const bool polarity){
    auto reg = RegCopy(regs_.wire_config_reg);
    reg.pwm_polarity_low = !polarity;
    return write_reg(reg);
}

IResult<> MT6701::set_pwm_freq(const PwmFreq pwm_freq){
    auto reg = RegCopy(regs_.wire_config_reg);
    reg.pwm_freq = pwm_freq;
    return write_reg(reg);
}

IResult<> MT6701::set_start_angle(const Angular<uq32> angle){

    const uint16_t bits = perunit_angle_to_u12(angle);
    {
        auto reg =  RegCopy(regs_.start_reg);
        reg.bits = static_cast<uint8_t>(bits & 0xff);
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regs_.start_stop_reg);
        reg.start = static_cast<uint8_t>(bits >> 8);
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }
    return Ok();
}

IResult<> MT6701::set_stop_angle(const Angular<uq32> angle){
    const uint16_t bits = perunit_angle_to_u12(angle);

    {
        auto reg = RegCopy(regs_.stop_reg);
        reg.bits = static_cast<uint8_t>(bits & 0xff);
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regs_.start_stop_reg);
        reg.stop = static_cast<uint8_t>(bits >> 8);
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }
    return Ok();
}
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




static constexpr uint16_t uni_to_u12(const real_t uni){
    return uint16_t(uni * 4096);
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
    const auto res = read_reg(raw_angle_reg);
    lap_position_ = uq32::from_bits(raw_angle_reg.angle);
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
    auto reg = RegCopy(uvw_mux_reg);
    reg.uvw_mux = en == EN;
    return write_reg(reg);
}

IResult<> MT6701::enable_abzmux(const Enable en){
    auto reg = RegCopy(abz_mux_reg);
    reg.abz_mux = en == EN;
    return write_reg(reg);
}

IResult<> MT6701::set_direction(const bool clockwise){
    auto reg = RegCopy(abz_mux_reg);
    reg.clockwise = clockwise;
    return write_reg(reg);
}

IResult<> MT6701::set_pole_pairs(const uint8_t pole_pairs){
    auto reg = RegCopy(resolution_reg);
    reg.pole_pairs = pole_pairs;
    return write_reg(reg);
}

IResult<> MT6701::set_abz_resolution(const uint16_t abz_resolution){

    auto reg = RegCopy(resolution_reg);
    reg.abz_resolution = abz_resolution;
    return write_reg(reg);
}

IResult<> MT6701::set_zero_position(
        const uint16_t zero_position){

    auto reg = RegCopy(zero_config_reg);
    reg.zero_position = zero_position;
    return write_reg(reg);
}

IResult<> MT6701::set_zero_pulse_width(
        const ZeroPulseWidth zero_pulse_width){

    auto reg = RegCopy(zero_config_reg);
    reg.zero_pulse_width = zero_pulse_width;
    return write_reg(reg);
}

IResult<> MT6701::set_hysteresis(const Hysteresis hysteresis){
    {
        auto reg = RegCopy(hystersis_reg);
        reg.hysteresis = static_cast<uint8_t>(hysteresis)  & 0b11;
        if(const auto res = write_reg(reg);
            res.is_err()) return res;
    }
    {
        auto reg = RegCopy(zero_config_reg);
        reg.hysteresis = static_cast<uint8_t>(hysteresis) >> 2;
        return write_reg(reg);
    }
}

IResult<> MT6701::enable_fast_mode(const Enable en){
    fast_mode_ = en == EN;
    return Ok();
}

IResult<> MT6701::enable_pwm(const Enable en){
    auto reg = RegCopy(wire_config_reg);
    reg.isPwm = en == EN;
    return write_reg(reg);
}

IResult<> MT6701::set_pwm_polarity(const bool polarity){
    auto reg = RegCopy(wire_config_reg);
    reg.pwm_polarity_low = !polarity;
    return write_reg(reg);
}

IResult<> MT6701::set_pwm_freq(const PwmFreq pwm_freq){
    auto reg = RegCopy(wire_config_reg);
    reg.pwm_freq = pwm_freq;
    return write_reg(reg);
}

IResult<> MT6701::set_start_position(const real_t start){

    const uint16_t start_data = uni_to_u12(start);
    {
        auto reg =  RegCopy(start_reg);
        reg.bits = start_data;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(start_stop_reg);
        reg.start = start_data >> 8;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }
    return Ok();
}

IResult<> MT6701::set_stop_position(const real_t stop){
    const uint16_t stop_data = uni_to_u12(stop);

    {
        auto reg = RegCopy(stop_reg);
        reg.bits = stop_data;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(start_stop_reg);
        reg.stop = stop_data >> 8;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }
    return Ok();
}
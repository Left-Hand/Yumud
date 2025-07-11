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
    const auto res = phy_.read_reg(raw_angle_reg);
    lap_position_ = u16_to_uni(raw_angle_reg.angle);
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
    //         lap_position = real_t(iq_t<16>(semantic.data_14bit << 2) >> 16);
    //     } 
    // }
    // else{
    //     MT6701_DEBUG("no drv!!");
    //     PANIC();
    // }
}

IResult<real_t> MT6701::read_lap_position(){
    return Ok(lap_position_);
}


IResult<> MT6701::enable_uvwmux(const Enable en){
    auto reg = RegCopy(uvw_mux_reg);
    reg.uvwMux = en == EN;
    return phy_.write_reg(reg);
}

IResult<> MT6701::enable_abzmux(const Enable en){
    auto reg = RegCopy(abz_mux_reg);
    reg.abzMux = en == EN;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_direction(const bool clockwise){
    auto reg = RegCopy(abz_mux_reg);
    reg.clockwise = clockwise;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_poles(const uint8_t _poles){
    auto reg = RegCopy(resolution_reg);
    reg.poles = _poles;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_abz_resolution(const uint16_t abzResolution){

    auto reg = RegCopy(resolution_reg);
    reg.abzResolution = abzResolution;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_zero_position(
        const uint16_t zeroPosition){

    auto reg = RegCopy(zero_config_reg);
    reg.zeroPosition = zeroPosition;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_zero_pulse_width(
        const ZeroPulseWidth zeroPulseWidth){

    auto reg = RegCopy(zero_config_reg);
    reg.zeroPulseWidth = (uint8_t)zeroPulseWidth;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_hysteresis(const Hysteresis hysteresis){
    {
        auto reg = RegCopy(hystersis_reg);
        reg.hysteresis = static_cast<uint8_t>(hysteresis)  & 0b11;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return res;
    }
    {
        auto reg = RegCopy(zero_config_reg);
        reg.hysteresis = static_cast<uint8_t>(hysteresis) >> 2;
        return phy_.write_reg(reg);
    }
}

IResult<> MT6701::enable_fast_mode(const Enable en){
    fast_mode_ = en == EN;
    return Ok();
}

IResult<> MT6701::enable_pwm(const Enable en){
    auto reg = RegCopy(wire_config_reg);
    reg.isPwm = en == EN;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_pwm_polarity(const bool polarity){
    auto reg = RegCopy(wire_config_reg);
    reg.pwmPolarityLow = !polarity;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_pwm_freq(const PwmFreq pwmFreq){
    auto reg = RegCopy(wire_config_reg);
    reg.pwmFreq = (uint8_t)pwmFreq;
    return phy_.write_reg(reg);
}

IResult<> MT6701::set_start_position(const real_t start){

    const uint16_t start_data = uni_to_u12(start);
    {
        auto reg =  RegCopy(start_reg);
        reg.data = start_data;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(start_stop_reg);
        reg.start = start_data >> 8;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }
    return Ok();
}

IResult<> MT6701::set_stop_position(const real_t stop){
    const uint16_t stop_data = uni_to_u12(stop);

    {
        auto reg = RegCopy(stop_reg);
        reg.data = stop_data;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(start_stop_reg);
        reg.stop = stop_data >> 8;
        if(const auto res = phy_.write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }
    return Ok();
}
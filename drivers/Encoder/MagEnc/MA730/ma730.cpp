#include "ma730.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define MA730_DEBUG_EN

#ifdef MA730_DEBUG_EN
#define MA730_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MA730_PANIC(...) PANIC(__VA_ARGS__)
#define MA730_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define MA730_DEBUG(...)
#define MA730_PANIC(...)  PANIC()
#define MA730_ASSERT(cond, ...) ASSERT(cond)
#endif

using Error = MA730::Error;
template<typename T = void>
using IResult = typename MA730::IResult<T>;

IResult<> MA730::init(const Config & cfg){
    if(const auto res = set_direction(cfg.direction);
        unlikely(res.is_err())) return Err(res.unwrap_err());
    if(const auto res = read_lap_position();
        unlikely(res.is_err())) return Err(res.unwrap_err());
    return Ok();
}

IResult<uint16_t> MA730::direct_read(){
    uint16_t data;
    const auto res = spi_drv_.read_single<uint16_t>(data);
    if(unlikely(res.is_err())) return Err(Error(res.unwrap_err()));
    return Ok(data);
}


IResult<uint16_t> MA730::get_raw_data(){
    return direct_read();
}


IResult<> MA730::set_zero_data(const uint16_t data){
    {
        auto reg = RegCopy(zero_data_low_reg);
        reg.data = data & 0xff;
        return write_reg(reg);
    }

    {
        auto reg = RegCopy(zero_data_high_reg);
        reg.data = data >> 8;
        return write_reg(reg);
    }
}


IResult<> MA730::set_zero_position(const real_t position){
    const auto data = uni_to_u16(frac(position));
    return set_zero_data(data);
}

IResult<MagStatus> MA730::get_mag_status(){
    const auto res = read_reg(magnitude_reg);
    if(unlikely(res.is_err())) return Err(res.unwrap_err());

    const bool mgl = !(magnitude_reg.mgl1 | magnitude_reg.mgl2);
    const bool mgh = magnitude_reg.magnitudeHigh;

    if(mgl) return Ok(MagStatus::Low());
    else if(mgh) return Ok(MagStatus::High());
    else return Ok(MagStatus::Proper());
}

IResult<> MA730::update(){
    const uint16_t data = ({
        const auto res = direct_read();
        if(unlikely(res.is_err())) return Err(res.unwrap_err());
        res.unwrap();
    });
    lap_position_ = u16_to_uni(data);
    return Ok();
}


IResult<> MA730::set_trim_x(const real_t k){
    {
        auto reg = RegCopy(trim_reg);
        reg.trim = uint8_t((real_t(1) - real_t(1) / k) * 258);
        return write_reg(reg);
    }
    {
        auto reg = RegCopy(trim_config_reg);
        reg.enableX = true;
        reg.enableY = false;
        return write_reg(reg);
    }
}

IResult<> MA730::set_trim_y(const real_t k){
    {
        auto reg = RegCopy(trim_reg);
        reg.trim = uint8_t((1.0_r - k) * 258);
        return write_reg(reg);
    }
    {
        auto reg = RegCopy(trim_config_reg);
        reg.enableX = false;
        reg.enableY = true;
        return write_reg(reg);
    }
}


IResult<> MA730::set_trim(const real_t am, const real_t e){
    real_t k = std::tan(am + e) / std::tan(am);
    if(k > real_t(1)) return set_trim_x(k);
    else return set_trim_y(k);
}

IResult<> MA730::set_mag_threshold(const MagThreshold low, const MagThreshold high){
    auto reg = RegCopy(threshold_reg);
    reg.thresholdLow = uint8_t(low);
    reg.thresholdHigh = uint8_t(high);
    return write_reg(reg);
}

IResult<> MA730::set_direction(const ClockDirection direction){
    auto reg = RegCopy(direction_reg);
    reg.direction = direction == CCW;
    return write_reg(reg);
}


IResult<> MA730::set_zparameters(const Width width, const Phase phase){
    auto reg = RegCopy(z_parameters_reg);
    reg.zWidth = width;
    reg.zPhase = phase;
    return write_reg(reg);
}

IResult<> MA730::set_pulse_per_turn(uint16_t ppt){
    const auto ppt_u10 = CLAMP(ppt - 1, 0, 1023);

    {
        const uint8_t ppt_l = ppt_u10 & 0b11;

        auto reg = RegCopy(z_parameters_reg);
        reg.ppt = ppt_l;

        if(const auto res = (write_reg(reg));
            unlikely(res.is_err())) return Err(res.unwrap_err()); 
    }

    {
        const uint8_t ppt_h = ppt_u10 >> 2;

        auto reg = RegCopy(pulse_per_turn_reg);
        reg.data = ppt_h;

        if(const auto res = (write_reg(reg));
            unlikely(res.is_err())) return Err(res.unwrap_err());
    }

    return Ok();

}

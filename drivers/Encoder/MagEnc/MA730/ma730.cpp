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
    if(const auto res = read_lap_angle();
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
        auto reg = RegCopy(regs_.zero_data_low_reg);
        reg.data = data & 0xff;
        return write_reg(reg);
    }

    {
        auto reg = RegCopy(regs_.zero_data_high_reg);
        reg.data = data >> 8;
        return write_reg(reg);
    }
}


IResult<> MA730::set_zero_angle(const Angle<q31> angle){
    const auto data = uni_to_u16(static_cast<q16>(angle.to_turns()));
    return set_zero_data(data);
}

IResult<MagStatus> MA730::get_mag_status(){
    auto & reg = regs_.magnitude_reg;
    const auto res = read_reg(reg);
    if(unlikely(res.is_err())) return Err(res.unwrap_err());

    const bool mgl = !(reg.mgl1 | reg.mgl2);
    const bool mgh = reg.magnitudeHigh;

    if(mgl) return Ok(MagStatus::from_low());
    else if(mgh) return Ok(MagStatus::from_high());
    else return Ok(MagStatus::from_proper());
}

IResult<> MA730::update(){
    const uint16_t data = ({
        const auto res = direct_read();
        if(unlikely(res.is_err())) return Err(res.unwrap_err());
        res.unwrap();
    });
    lap_angle_ = u16_to_uni(data);
    return Ok();
}


IResult<> MA730::set_trim_x(const real_t k){
    {
        auto reg = RegCopy(regs_.trim_reg);
        reg.trim = uint8_t((real_t(1) - real_t(1) / k) * 258);
        return write_reg(reg);
    }
    {
        auto reg = RegCopy(regs_.trim_config_reg);
        reg.enableX = true;
        reg.enableY = false;
        return write_reg(reg);
    }
}

IResult<> MA730::set_trim_y(const real_t k){
    {
        auto reg = RegCopy(regs_.trim_reg);
        reg.trim = uint8_t((1.0_r - k) * 258);
        return write_reg(reg);
    }
    {
        auto reg = RegCopy(regs_.trim_config_reg);
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
    auto reg = RegCopy(regs_.threshold_reg);
    reg.low = low;
    reg.high = high;
    return write_reg(reg);
}

IResult<> MA730::set_direction(const ClockDirection direction){
    auto reg = RegCopy(regs_.direction_reg);
    reg.direction = direction == CCW;
    return write_reg(reg);
}


IResult<> MA730::set_zparameters(const PulseWidth width, const PulsePhase phase){
    auto reg = RegCopy(regs_.z_parameters_reg);
    reg.zWidth = width;
    reg.zPhase = phase;
    return write_reg(reg);
}

IResult<> MA730::set_pulse_per_turn(uint16_t ppt){
    const auto ppt_u10 = CLAMP(ppt - 1, 0, 1023);

    {
        const uint8_t ppt_l = ppt_u10 & 0b11;

        auto reg = RegCopy(regs_.z_parameters_reg);
        reg.ppt = ppt_l;

        if(const auto res = (write_reg(reg));
            unlikely(res.is_err())) return Err(res.unwrap_err()); 
    }

    {
        const uint8_t ppt_h = ppt_u10 >> 2;

        auto reg = RegCopy(regs_.pulse_per_turn_reg);
        reg.data = ppt_h;

        if(const auto res = (write_reg(reg));
            unlikely(res.is_err())) return Err(res.unwrap_err());
    }

    return Ok();

}

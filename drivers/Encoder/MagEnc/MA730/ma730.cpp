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
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_lap_angle();
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<uint16_t> MA730::direct_read(){
    uint16_t bits;
    const auto res = spi_drv_.read_single<uint16_t>(bits);
    if(res.is_err()) return Err(Error(res.unwrap_err()));
    return Ok(bits);
}


IResult<uint16_t> MA730::get_raw_data(){
    return direct_read();
}



IResult<> MA730::set_zero_angle(const Angular<uq32> angle){
    const auto bits = (angle.to_turns().to_bits() >> 16);
    {
        auto reg = RegCopy(regs_.zero_low_reg);
        reg.bits = bits & 0xff;
        return write_reg(reg);
    }

    {
        auto reg = RegCopy(regs_.zero_high_reg);
        reg.bits = bits >> 8;
        return write_reg(reg);
    }
}

IResult<EncoderFaultBitFields> MA730::get_fault(){
    auto & reg = regs_.magnitude_reg;
    const auto res = read_reg(reg);
    if(res.is_err()) return Err(res.unwrap_err());

    const bool mgl = !(reg.mgl1 | reg.mgl2);
    const bool mgh = reg.magnitude_high;

    EncoderFaultBitFields fault = EncoderFaultBitFields::zero();
    if(mgl){
        fault.mag_strength = EncoderFaultBitFields::MagStrength::Low;
    }else if(mgh){
        fault.mag_strength = EncoderFaultBitFields::MagStrength::High;
    } else{
        fault.mag_strength = EncoderFaultBitFields::MagStrength::Proper;
    }
    return Ok(fault);
}

IResult<> MA730::update(){
    const uint16_t data = ({
        const auto res = direct_read();
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });
    lap_turns_ = static_cast<uq32>(uq16::from_bits(data));
    return Ok();
}


IResult<> MA730::set_trim_x(const uq16 k){
    {
        auto reg = RegCopy(regs_.trim_reg);
        reg.trim = uint8_t((uq16(1) - uq16(1) / k) * 258);
        return write_reg(reg);
    }
    {
        auto reg = RegCopy(regs_.trim_config_reg);
        reg.enable_x = true;
        reg.enable_y = false;
        return write_reg(reg);
    }
}

IResult<> MA730::set_trim_y(const uq16 k){
    {
        auto reg = RegCopy(regs_.trim_reg);
        reg.trim = uint8_t((1.0_r - k) * 258);
        return write_reg(reg);
    }
    {
        auto reg = RegCopy(regs_.trim_config_reg);
        reg.enable_x = false;
        reg.enable_y = true;
        return write_reg(reg);
    }
}


IResult<> MA730::set_trim(const uq16 am, const uq16 e){
    uq16 k = std::tan(iq16(am + e)) / std::tan(iq16(am));
    if(k > uq16(1)) return set_trim_x(k);
    else return set_trim_y(k);
}

IResult<> MA730::set_mag_threshold(const MagThreshold low, const MagThreshold high){
    auto reg = RegCopy(regs_.threshold_reg);
    reg.low = low;
    reg.high = high;
    return write_reg(reg);
}

IResult<> MA730::set_direction(const RotateDirection direction){
    auto reg = RegCopy(regs_.direction_reg);
    reg.direction_is_ccw = direction == CCW;
    return write_reg(reg);
}


IResult<> MA730::set_zero_parameters(const ZeroPulseWidth width, const ZeroPulsePhase phase){
    auto reg = RegCopy(regs_.z_parameters_reg);
    reg.z_width = width;
    reg.z_phase = phase;
    return write_reg(reg);
}

IResult<> MA730::set_pulse_per_turn(uint16_t ppt){
    const auto ppt_u10 = CLAMP(ppt - 1, 0, 1023);

    {
        const uint8_t ppt_l = ppt_u10 & 0b11;

        auto reg = RegCopy(regs_.z_parameters_reg);
        reg.ppt = ppt_l;

        if(const auto res = (write_reg(reg));
            res.is_err()) return Err(res.unwrap_err()); 
    }

    {
        const uint8_t ppt_h = ppt_u10 >> 2;

        auto reg = RegCopy(regs_.pulse_per_turn_reg);
        reg.bits = ppt_h;

        if(const auto res = (write_reg(reg));
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();

}

#include "ma730.hpp"
#include "core/math/realmath.hpp"

using namespace ymd::drivers;
using namespace ymd;

#define MA730_DEBUG

#ifdef MA730_DEBUG
#undef MA730_DEBUG
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

void MA730::init(){
    get_lap_position();
}

IResult<> MA730::write_reg(const RegAddress addr, uint8_t data){
    const auto res = spi_drv_.write_single<uint16_t>((uint16_t)(0x8000 | ((uint8_t)addr << 8) | data));
    if(res.is_err()) return Err(Error(res.unwrap_err()));
    return Ok();
}

IResult<> MA730::read_reg(const RegAddress addr, uint8_t & data){
    uint16_t dummy;
    if(const auto res = spi_drv_.write_single<uint16_t>((uint16_t)(0x4000 | ((uint8_t)addr << 8))); res.is_err())
        return Err(Error(res.unwrap_err()));
    if(const auto res = spi_drv_.read_single<uint16_t>(dummy); res.is_err()) 
        return Err(Error(res.unwrap_err()));
    data = dummy >> 8;
    return Ok();
}

IResult<> MA730::direct_read(uint16_t & data){
    const auto res = spi_drv_.read_single<uint16_t>(data);
    if(res.is_err()) return Err(Error(res.unwrap_err()));
    return Ok();
}


IResult<uint16_t> MA730::get_raw_data(){
    uint16_t data = 0;
    if(const auto res = direct_read(data); res.is_err())
        return Err(res.unwrap_err());
    return Ok(data);
}


IResult<> MA730::set_zero_data(const uint16_t data){
    zeroDataReg = data & 0xff;
    return write_reg(RegAddress::ZeroDataLow, zeroDataReg & 0xff) | 
    write_reg(RegAddress::ZeroDataHigh, zeroDataReg >> 8);
}


IResult<> MA730::set_zero_position(const real_t position){
    const auto data = uni_to_u16(frac(position));
    return set_zero_data(data);
}


IResult<> MA730::update(){
    uint16_t data = 0;
    const auto res = direct_read(data);
    lap_position = u16_to_uni(data);
    return res;
}


IResult<> MA730::set_trim_x(const real_t k){
    trimReg = (uint8_t)((real_t(1) - real_t(1) / k) * 258);
    trimConfigReg.enableX = true;
    trimConfigReg.enableY = false;

    return write_reg(RegAddress::Trim, trimReg)
    | write_reg(RegAddress::TrimConfig, uint8_t(trimConfigReg));
}

IResult<> MA730::set_trim_y(const real_t k){
    trimReg = (uint8_t)((real_t(1) - k) * 258);
    trimConfigReg.enableX = false;
    trimConfigReg.enableY = true;

    return write_reg(RegAddress::Trim, trimReg)
    | write_reg(RegAddress::TrimConfig, uint8_t(trimConfigReg))
    ;
}


IResult<> MA730::set_trim(const real_t am, const real_t e){
    real_t k = std::tan(am + e) / std::tan(am);
    if(k > real_t(1)) return set_trim_x(k);
    else return set_trim_y(k);
}

IResult<> MA730::set_mag_threshold(const MagThreshold low, const MagThreshold high){
    auto & reg = thresholdReg;
    reg.thresholdLow = uint8_t(low);
    reg.thresholdHigh = uint8_t(high);
    return write_reg(RegAddress::Threshold, reg.as_val())
        // .inspect([&](auto && x){org_reg = reg;})
    ;
}

IResult<> MA730::set_direction(const bool direction){
    directionReg.direction = direction;
    return write_reg(RegAddress::Direction, uint8_t(directionReg));
}

IResult<bool> MA730::is_magnitude_low(){
    const auto res = read_reg(RegAddress::Magnitude, magnitudeReg);
    if(res.is_err()) return Err(res.unwrap_err());
    bool correctMgl = !(magnitudeReg.mgl1 | magnitudeReg.mgl2);
    return Ok(bool(correctMgl));
}

IResult<bool> MA730::is_magnitude_high(){
    const auto res = read_reg(RegAddress::Magnitude, magnitudeReg);
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(magnitudeReg.magnitudeHigh));
}

IResult<bool> MA730::is_magnitude_proper(){
    const auto res = read_reg(RegAddress::Magnitude, magnitudeReg);
    if(res.is_err()) return Err(res.unwrap_err());
    const bool proper = !((!(magnitudeReg.mgl1 | magnitudeReg.mgl2)) | magnitudeReg.magnitudeHigh);
    return Ok(proper);
}

IResult<> MA730::set_zparameters(const Width width, const Phase phase){
    zParametersReg.zWidth = (uint8_t)width;
    zParametersReg.zPhase = (uint8_t)phase;
    return write_reg(RegAddress::ZParameters, uint8_t(zParametersReg));
}

IResult<> MA730::set_pulse_per_turn(uint16_t ppt){
    ppt = CLAMP(ppt - 1, 0, 1023);
    uint8_t ppt_l = ppt & 0b11;
    uint8_t ppt_h = ppt >> 2;
    
    zParametersReg.ppt = ppt_l;
    pulsePerTurnReg = ppt_h;

    return (write_reg(RegAddress::ZParameters, uint8_t(zParametersReg)))
    | (write_reg(RegAddress::PulsePerTurn, pulsePerTurnReg));
}

#include "tcs34725.hpp"


#define TCS34725_DEBUG_EN 0

#if TCS34725_DEBUG_EN
#define TCS34725_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define TCS34725_PANIC(...) PANIC(__VA_ARGS__)
#define TCS34725_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define TCS34725_DEBUG(...)
#define TCS34725_PANIC(...)  PANIC()
#define TCS34725_ASSERT(cond, ...) ASSERT(cond)
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = TCS34725::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> TCS34725::read_burst(
    const TCS34725::RegAddr addr, 
    const std::span<uint16_t> pbuf
){
    uint8_t address = conv_reg_address_repeated(addr);
    if(const auto res = i2c_drv_.read_burst(address, pbuf, std::endian::little);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


std::tuple<uq16, uq16, uq16, uq16> TCS34725::get_crgb(){
    return {
        uq16::from_bits(crgb_[0]),
        uq16::from_bits(crgb_[1]),
        uq16::from_bits(crgb_[2]),
        uq16::from_bits(crgb_[3])
    };
}

IResult<> TCS34725::update(){
    return read_burst(RegAddr::ClearData, std::span(crgb_));
}


IResult<> TCS34725::validate(){
    auto & reg = regs_.device_id_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    if(reg.id != reg.KEY)
        return Err(Error::WrongChipId);
    return Ok();
}

IResult<> TCS34725::set_int_persistence(const uint8_t times){
    auto reg = RegCopy(regs_.int_persistence_reg);
    if(times >= 5){
        uint8_t value = 0b0100 + (times / 5) - 1;
        reg.apers = value;
    }else{
        reg.apers = (uint8_t)MIN(times, 3);
    }

    return write_reg(reg);
}


IResult<> TCS34725::set_integration_time(const Milliseconds ms){
    const uint16_t cycles = CLAMP(ms.count() * 10 / 24, 1, 256);
    const uint16_t temp = 256 - cycles;
    auto reg = RegCopy(regs_.integration_reg);
    reg.data = temp;
    return write_reg(reg);
}

IResult<> TCS34725::set_wait_time(const Milliseconds ms){
    const uint16_t ms_l = MAX(ms.count() * 10 / 24,1);
    uint16_t value;
    bool long_wait_flag = false;

    if(ms_l <= 256){
        value = 256 - ms_l;
    }else{
        uint16_t ms_h = CLAMP(ms.count() * 10 / 24 / 12, 1, 256);
        value = 256 - ms_h;
        long_wait_flag = true;
    }

    {
        auto reg = RegCopy(regs_.wait_time_reg);
        reg.data = value;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regs_.long_wait_reg);
        reg.wait_long = long_wait_flag;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

IResult<> TCS34725::set_int_thr_low(const uint16_t thr){
    auto reg = RegCopy(regs_.low_thr_reg);
    reg.data = thr;
    return write_reg(reg);
}

IResult<> TCS34725::set_int_thr_high(const uint16_t thr){
    auto reg = RegCopy(regs_.high_thr_reg);
    reg.data = thr;
    return write_reg(reg);
}

IResult<> TCS34725::set_int_persistence(const uint8_t times);

IResult<> TCS34725::set_gain(const Gain gain){
    auto reg = RegCopy(regs_.gain_reg);
    reg.gain = gain;
    return write_reg(reg);
}

IResult<uint8_t> TCS34725::get_id(){
    auto & reg = regs_.device_id_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.to_bits());
}

IResult<bool> TCS34725::is_idle(){
    auto & reg = regs_.status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(uint8_t(reg.done_flag));
}

IResult<> TCS34725::set_power(const bool on){
    auto reg = RegCopy(regs_.enable_reg);
    reg.powerOn = on;
    return write_reg(reg);
}

IResult<> TCS34725::start_conv(){
    auto reg = RegCopy(regs_.enable_reg);

    reg.adc_en = true;
    return write_reg(reg);
}


IResult<> TCS34725::init(const Config & cfg){
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = set_power(true);
        res.is_err()) return res;
    if(const auto res = set_integration_time(cfg.integration_time);
        res.is_err()) return res;
    if(const auto res = set_gain(cfg.gain);
        res.is_err()) return res;
    return Ok();
}
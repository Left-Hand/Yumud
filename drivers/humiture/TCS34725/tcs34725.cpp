#include "tcs34725.hpp"


#define TCS34725_DEBUG_EN 0

#if TCS34725_DEBUG_EN
#define TCS34725_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define TCS34725_PANIC(...) PANIC(__VA_ARGS__)
#define TCS34725_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define TCS34725_DEBUG(...)
#define TCS34725_PANIC(...)  PANIC_NSRC()
#define TCS34725_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif


using namespace ymd;
using namespace ymd::drivers;

using Error = TCS34725::Error;

template<typename T = void>
using IResult = Result<T, Error>;



IResult<TCS34725::Crgb16> TCS34725::get_crgb(){
    Crgb16 crgb;
    if(const auto res = read_bulk(RegAddr::ClearData, std::span(crgb.elements));
        res.is_err()) return Err(res.unwrap_err());
    return Ok(crgb);
}


IResult<TCS34725::Package> TCS34725::validate(){
    auto reg = Regs::R8_DeviceId{};
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    switch(std::bit_cast<Package>(reg.id)){
        case Package::TCS34725:
        case Package::TCS34723:
            return Ok(std::bit_cast<Package>(reg.id));
    }
    return Err(Error::InvalidChipId);
}

IResult<> TCS34725::set_int_persistence(const uint8_t times){
    auto reg = RegCopy(regs_.int_persistence_reg);
    if(times >= 5){
        uint8_t value = 0b0100 + (times / 5) - 1;
        reg.apers = value;
    }else{
        reg.apers = std::min<uint8_t>(times, 3);
    }

    return write_reg(reg);
}


IResult<> TCS34725::set_integration_time(const Milliseconds ms){
    const uint16_t cycles = std::clamp(int(ms.count() * 10 / 24), 1, 256);
    const uint16_t temp = 256 - cycles;
    auto reg = RegCopy(regs_.integration_reg);
    reg.data = temp;
    return write_reg(reg);
}


static constexpr auto ms_to_waittime(
    const uint32_t ms
){

    struct Ret{
        uint8_t wait_time;
        bool long_wait_flag;
    };

    Ret ret;

    const uint16_t ms_l = std::max(int(ms * 10 / 24),1);

    if(ms_l <= 256){
        ret.wait_time = 256 - ms_l;
    }else{
        uint16_t ms_h = std::clamp(int(ms * 10 / 24 / 12), 1, 256);
        ret.wait_time = 256 - ms_h;
        ret.long_wait_flag = true;
    }

    return ret;
}
IResult<> TCS34725::set_wait_time(const Milliseconds ms){
    const auto parts = ms_to_waittime(ms.count());
    {
        auto reg = RegCopy(regs_.wait_time_reg);
        reg.wait_time = parts.wait_time;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regs_.long_wait_reg);
        reg.wait_long = parts.long_wait_flag;
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

IResult<> TCS34725::set_gain(const Gain gain){
    auto reg = RegCopy(regs_.gain_reg);
    reg.gain = gain;
    return write_reg(reg);
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
        res.is_err()) Err(res.unwrap_err());
    if(const auto res = set_power(true);
        res.is_err()) Err(res.unwrap_err());
    if(const auto res = set_integration_time(cfg.integration_time);
        res.is_err()) Err(res.unwrap_err());
    if(const auto res = set_gain(cfg.gain);
        res.is_err()) Err(res.unwrap_err());
    return Ok();
}

IResult<> TCS34725::read_bulk(
    const TCS34725::RegAddr reg_addr, 
    const std::span<uint16_t> pbuf
){
    uint8_t address = conv_reg_address_repeated(reg_addr);
    if(const auto res = i2c_drv_.read_bulk(
        address, 
        pbuf, std::endian::little
    ); res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

#include "tcs34725.hpp"


#ifdef TCS34725_DEBUG_EN
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
    const TCS34725::RegAddress addr, 
    const std::span<uint16_t> pdata
){
    uint8_t address = conv_reg_address(addr);
    // TCS34725_DEBUG("address", address)

    if(const auto res = i2c_drv_.read_burst(address, pdata, LSB);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


std::tuple<real_t, real_t, real_t, real_t> TCS34725::get_crgb(){
    return {
        s16_to_uni(crgb[0]),
        s16_to_uni(crgb[1]),
        s16_to_uni(crgb[2]),
        s16_to_uni(crgb[3])
    };
}

IResult<> TCS34725::update(){
    return read_burst(RegAddress::ClearData, std::span(crgb));
}


IResult<> TCS34725::set_int_persistence(const uint8_t times){
    if(times >= 5){
        uint8_t value = 0b0100 + (times / 5) - 1;
        intPersistenceReg = value;
    }else{
        intPersistenceReg = (uint8_t)MIN(times, 3);
    }

    return write_reg(RegAddress::Integration, integrationReg);
}


IResult<> TCS34725::set_integration_time(const Milliseconds ms){
    const uint16_t cycles = CLAMP(ms.count() * 10 / 24, 1, 256);
    const uint16_t temp = 256 - cycles;
    integrationReg = temp;
    return write_reg(RegAddress::Integration, integrationReg);
}

IResult<> TCS34725::set_wait_time(const Milliseconds ms){
    const uint16_t ms_l = MAX(ms.count() * 10 / 24,1);
    uint16_t value;
    bool long_waitFlag = false;
    if(ms_l <= 256){
        value = 256 - ms_l;
    }else{
        uint16_t ms_h = CLAMP(ms.count() * 10 / 24 / 12, 1, 256);
        value = 256 - ms_h;
        long_waitFlag = true;
    }

    waitTimeReg = value;
    if(const auto res = write_reg(RegAddress::WaitTime, waitTimeReg);
        res.is_err()) return Err(res.unwrap_err());
    if(long_waitFlag){
        longWaitReg.waitLong = true;

        if(const auto res = write_reg(RegAddress::LongWait, longWaitReg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

IResult<> TCS34725::set_int_thr_low(const uint16_t thr){
    lowThrReg = thr;
    return write_reg(RegAddress::LowThr, lowThrReg);
}

IResult<> TCS34725::set_int_thr_high(const uint16_t thr){
    highThrReg = thr;
    return write_reg(RegAddress::HighThr, highThrReg);
}

IResult<> TCS34725::set_int_persistence(const uint8_t times);

IResult<> TCS34725::set_gain(const Gain gain){
    gainReg = (uint8_t)gain;
    return write_reg(RegAddress::Gain, gainReg);
}

IResult<uint8_t> TCS34725::get_id(){
    if(const auto res = read_reg(RegAddress::DeviceId, deviceIdReg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(uint8_t(deviceIdReg));
}

IResult<bool> TCS34725::is_idle(){
    if(const auto res = read_reg(RegAddress::Status, statusReg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(uint8_t(statusReg.done_flag));
}

IResult<> TCS34725::set_power(const bool on){
    enableReg.powerOn = on;
    return write_reg(RegAddress::Enable, enableReg);
}

IResult<> TCS34725::start_conv(){
    enableReg.adcEn = true;
    return write_reg(RegAddress::Enable, enableReg);
}


IResult<> TCS34725::init(){
    if(const auto res = set_power(true);
        res.is_err()) return res;
    if(const auto res = set_integration_time(240ms);
        res.is_err()) return res;
    if(const auto res = set_gain(Gain::X1);
        res.is_err()) return res;
    return Ok();
}
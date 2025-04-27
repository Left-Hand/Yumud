#include "tcs34725.hpp"


#ifdef TCS34725_DEBUG
#undef TCS34725_DEBUG
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

hal::BusError TCS34725::request_reg_data(const TCS34725::RegAddress addr, uint16_t * data_ptr, const size_t len){
    uint8_t address = conv_reg_address(addr);
    TCS34725_DEBUG("address", address)

    return i2c_drv_.read_burst(address, std::span(data_ptr, len), LSB);
}


std::tuple<real_t, real_t, real_t, real_t> TCS34725::get_crgb(){
    return {
        s16_to_uni(crgb[0]),
        s16_to_uni(crgb[1]),
        s16_to_uni(crgb[2]),
        s16_to_uni(crgb[3])
    };
}

void TCS34725::update(){
    request_reg_data(RegAddress::ClearData, crgb, 4);
}


void TCS34725::set_int_persistence(const uint8_t times){
    if(times >= 5){
        uint8_t value = 0b0100 + (times / 5) - 1;
        intPersistenceReg = value;
    }else{
        intPersistenceReg = (uint8_t)MIN(times, 3);
    }

    write_reg(RegAddress::Integration, integrationReg);
}

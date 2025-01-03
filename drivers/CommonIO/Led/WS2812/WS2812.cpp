#include "WS2812.hpp"
#include "sys/clock/clock.h"

using namespace ymd::drivers;

void WS2812::delayLong(){
    __nopn(120);
}

void WS2812::delayShort(){
    __nopn(32);
}

void WS2812::sendCode(const bool state){
    // __disable_irq();
    if(state){
        gpio.set();
        delayLong();
        gpio.clr();
        delayShort();
    }else{
        gpio.set();
        delayShort();
        gpio.clr();
        delayLong();
    }
    // __enable_irq();
}

void WS2812::sendByte(const uint8_t data){
    for(uint8_t mask = 0x80; mask; mask >>= 1){
        sendCode(data & mask);
    }
}

void WS2812::sendReset(){
    gpio.clr();
    delayMicroseconds(60);
}

void WS2812::_update(const Color &color){
    auto r = uni_to_u16(color.r);
    auto g = uni_to_u16(color.g);
    auto b = uni_to_u16(color.b);

    sendReset();
    sendByte(g >> 8);
    sendByte(r >> 8);
    sendByte(b >> 8);
}

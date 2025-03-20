#include "WS2812.hpp"
#include "core/clock/clock.hpp"
#include "hal/gpio/gpio_intf.hpp"

using namespace ymd::drivers;

void WS2812_Phy::delayLong(){
    __nopn(120);
}

void WS2812_Phy::delayShort(){
    __nopn(32);
}

void WS2812_Phy::sendCode(const bool state){
    // __disable_irq();
    if(state){
        gpio_.set();
        delayLong();
        gpio_.clr();
        delayShort();
    }else{
        gpio_.set();
        delayShort();
        gpio_.clr();
        delayLong();
    }
    // __enable_irq();
}

void WS2812_Phy::sendByte(const uint8_t data){
    for(uint8_t mask = 0x80; mask; mask >>= 1){
        sendCode(data & mask);
    }
}

void WS2812_Phy::sendReset(){
    gpio_.clr();
    delayMicroseconds(60);
}

void WS2812_Phy::init(){
    gpio_.outpp();
}

void WS2812::_update(const Color &color){
    auto r = uni_to_u16(color.r);
    auto g = uni_to_u16(color.g);
    auto b = uni_to_u16(color.b);

    phy_.sendReset();
    phy_.sendByte(g >> 8);
    phy_.sendByte(r >> 8);
    phy_.sendByte(b >> 8);
}

void WS2812::init(){
    phy_.init();
}

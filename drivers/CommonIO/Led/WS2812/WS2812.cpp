#include "WS2812.hpp"
#include "core/clock/clock.hpp"
#include "hal/gpio/gpio_intf.hpp"

#include "core/debug/debug.hpp"

using namespace ymd::drivers;

//short 0.4us	±150ns
//long 0.85us	±150ns

//144mhz

// #define DELAY_CELL __nopn(4)
#define DELAY_CELL clock::delay(1us);
void WS2812_Phy::delay_long(){
    for(size_t i = 0; i < 13; i++){
        DELAY_CELL;
    }
}

void WS2812_Phy::delay_short(){
    for(size_t i = 0; i < 4; i++){
        DELAY_CELL;
    }
}

void WS2812_Phy::send_code(const bool state){
    // __disable_irq();
    if(state){
        gpio_.set();
        delay_long();
        gpio_.clr();
        delay_short();
    }else{
        gpio_.set();
        delay_short();
        gpio_.clr();
        delay_long();
    }
    // __enable_irq();
}

void WS2812_Phy::send_byte(const uint8_t data){
    for(uint8_t mask = 0x80; mask; mask >>= 1){
        send_code(data & mask);
    }
}

void WS2812_Phy::send_reset(){
    gpio_.clr();
    clock::delay(60us);
}

void WS2812_Phy::init(){
    gpio_.outpp();
}

void WS2812::_update(const Color &color){
    uint8_t g = uint8_t(CLAMP(int(color.g * color.a * 256), 0, 255));
    uint8_t r = uint8_t(CLAMP(int(color.r * color.a * 256), 0, 255));
    uint8_t b = uint8_t(CLAMP(int(color.b * color.a * 256), 0, 255));
    

    phy_.send_reset();
    phy_.send_byte(g);
    phy_.send_byte(r);
    phy_.send_byte(b);

    // DEBUG_PRINTLN(g,r,b, color);
}

void WS2812::init(){
    phy_.init();
}

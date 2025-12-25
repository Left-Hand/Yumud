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
void WS2812_Transport::delay_long(){
    for(size_t i = 0; i < 13; i++){
        DELAY_CELL;
    }
}

void WS2812_Transport::delay_short(){
    for(size_t i = 0; i < 4; i++){
        DELAY_CELL;
    }
}

void WS2812_Transport::send_bit(const bool state){
    // __disable_irq();
    if(state){
        gpio_.set_high();
        delay_long();
        gpio_.set_low();
        delay_short();
    }else{
        gpio_.set_high();
        delay_short();
        gpio_.set_low();
        delay_long();
    }
    // __enable_irq();
}

void WS2812_Transport::send_byte(const uint8_t data){
    for(uint8_t mask = 0x80; mask; mask >>= 1){
        send_bit(data & mask);
    }
}

void WS2812_Transport::send_reset(){
    gpio_.set_low();
    clock::delay(60us);
}

void WS2812_Transport::init(){
    gpio_.outpp();
}

void WS2812::set_rgb(const RGB<iq16> &color){
    uint8_t g = uint8_t(CLAMP(int(color.g * 256), 0, 255));
    uint8_t r = uint8_t(CLAMP(int(color.r * 256), 0, 255));
    uint8_t b = uint8_t(CLAMP(int(color.b * 256), 0, 255));
    

    transport_.send_reset();
    transport_.send_byte(g);
    transport_.send_byte(r);
    transport_.send_byte(b);

    // DEBUG_PRINTLN(g,r,b, color);
}

void WS2812::init(){
    transport_.init();
}

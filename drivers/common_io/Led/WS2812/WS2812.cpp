#include "WS2812.hpp"
#include "core/clock/clock.hpp"
#include "hal/gpio/gpio_intf.hpp"

#include "core/debug/debug.hpp"

using namespace ymd::drivers;

//short 0.4us	±150ns
//long 0.85us	±150ns

//144mhz

// #define DELAY_CELL __nopn(4)
#define DELAY_CELL __nopn(6)
// #define DELAY_CELL clock::delay(1us);
void WS2812_Transport::delay_long(){
    for(size_t i = 0; i < 10; i++){
        DELAY_CELL;
    }
}

void WS2812_Transport::delay_short(){
    for(size_t i = 0; i < 4; i++){
        DELAY_CELL;
    }
}

void WS2812_Transport::send_bit(const bool b){
    if(b){
        pin.set_high();
        delay_long();
        pin.set_low();
        delay_short();
    }else{
        pin.set_high();
        delay_short();
        pin.set_low();
        delay_long();
    }
}

void WS2812_Transport::send_byte(const uint8_t b){
    for(uint8_t mask = 0x80; mask; mask >>= 1){
        send_bit(b & mask);
    }
}

void WS2812_Transport::send_reset(){
    pin.set_low();
    clock::delay(60us);
}

void WS2812_Transport::send_rgb888(
    const uint8_t r,
    const uint8_t g,
    const uint8_t b
){

    send_reset();
    send_byte(g);
    send_byte(r);
    send_byte(b);

}
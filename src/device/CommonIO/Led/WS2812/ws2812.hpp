#ifndef __WS2812_HPP__

#define __WS2812_HPP__

#include "../rgbLed.hpp"

class Ws2812: public RgbLedInst<false>{
protected:
    Gpio & gpio;

    void sendCode(const bool & state){
        if(state){
            gpio.set();
            __nopn(120);
            gpio.clr();
            __nopn(32);
        }else{
            gpio.set();
            __nopn(32);
            gpio.clr();
            __nopn(120);
        }
    }

    void sendReset(){
        gpio.clr();
        delayMicroseconds(60);
        gpio.set();
    }

    void sendByte(const uin8_t & data){
        for(uint8_t mask = 0x80; mask; mask >>= 1){
            sendCode(data & mask);
        }
    }
    void _update(const Color &color) override{
        uint16_t r,g,b;
        uni_to_u16(color.r, r);
        uni_to_u16(color.g, g);
        uni_to_u16(color.b, b);

        sendReset();
        sendByte(r >> 8);
        sendByte(b >> 8);
        sendByte(g >> 8);
    }

public:
    Ws2812(Gpio & _gpio):gpio(_gpio){;}
};
#endif
#ifndef __WS2812_HPP__

#define __WS2812_HPP__

#include "../rgbLed.hpp"

class WS2812: public RgbLedConcept<true>{
protected:
    Gpio gpio;

    void delayLong(){
        __nopn(120);
    }

    void delayShort(){
        __nopn(32);
    }
    void sendCode(const bool & state){
        __disable_irq();
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
        __enable_irq();
    }

    void sendReset(){
        gpio.clr();
        delayMicroseconds(60);
    }

    void sendByte(const uint8_t & data){
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
        sendByte(g >> 8);
        sendByte(r >> 8);
        sendByte(b >> 8);

        // gpio.set();
    }

public:
    WS2812(Gpio _gpio):gpio(_gpio){;}
    void init(){
        // gpio.set();
        gpio.OutPP();
    }
    WS2812 & operator = (const Color & color) override{
        setColor(color);
        return *this;
    }
};
#endif
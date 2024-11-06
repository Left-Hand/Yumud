#pragma once

#include "../RgbLed.hpp"
#include <array>

namespace yumud::drivers{

class WS2812: public RgbLedConcept{
protected:
    Gpio & gpio;

    __no_inline void delayLong();
    __no_inline void delayShort();

    void sendCode(const bool state);
    void sendByte(const uint8_t data);

    void sendReset();


    void _update(const Color &color);
public:
    WS2812(Gpio & _gpio):gpio(_gpio){;}
    void init(){
        gpio.outpp();
    }
    WS2812 & operator = (const Color & color) override{
        _update(color);
        return *this;
    }
};

class WS2812Single: public RgbLedConcept{
protected:
    using Color = Color_t<real_t>;


    void _update(const Color & _color){
        color = _color;
    }
public:
    Color color;
    WS2812Single() = default;

    WS2812Single & operator = (const Color & _color) override{
        _update(_color);
        return *this;
    }
};


template<size_t N>
class WS2812Chain{
protected:
    using Color = Color_t<real_t>;
    Gpio & gpio;
    std::array<WS2812Single, N> leds;

    void delayLong(){
        __nopn(120);
    }

    void delayShort(){
        __nopn(32);
    }
    void sendCode(const bool state){
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
        __nopn(4);
    }

    void sendReset(){
        gpio.clr();
        delayMicroseconds(60);
    }

    void sendByte(const uint8_t data){
        for(uint8_t mask = 0x80; mask; mask >>= 1){
            sendCode(data & mask);
        }
    }

public:
    WS2812Chain(Gpio & _gpio):gpio(_gpio){;}
    void init(){
        for(auto & led : leds) led = Color(0,0,0);
        gpio.outpp();
    }

    WS2812Single & operator[](const int index){
        if(index < 0) return leds.at(N + index);
        else return leds.at(index);
    }

    void refresh(){

        sendReset();

        for(auto & led : leds){
            uint16_t r,g,b;

            uni_to_u16(led.color.r, r);
            uni_to_u16(led.color.g, g);
            uni_to_u16(led.color.b, b);

            sendByte(g >> 8);
            sendByte(r >> 8);
            sendByte(b >> 8);

        }

    }
};

};

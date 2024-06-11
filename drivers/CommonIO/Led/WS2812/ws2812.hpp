#ifndef __WS2812_HPP__

#define __WS2812_HPP__

#include "../rgbLed.hpp"
#include <array>

class WS2812: public RgbLedConcept<true>{
protected:
    Gpio & gpio;

    void delayLong(){
        __nopn(120);
    }

    void delayShort(){
        __nopn(32);
    }
    void sendCode(const bool & state){
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
    }

public:
    WS2812(Gpio & _gpio):gpio(_gpio){;}
    void init(){
        // gpio.set();
        gpio.OutPP();
    }
    WS2812 & operator = (const Color & color) override{
        setColor(color);
        return *this;
    }
};

class WS2812Single: public RgbLedConcept<true>{
protected:
    using Color = Color_t<real_t>;


    void _update(const Color & _color) override{
        color = _color;
    }
public:
    Color color;
    WS2812Single() = default;
    void init() override{;}

    WS2812Single & operator = (const Color & _color) override{
        setColor(_color);
        return *this;
    }
};


template<uint16_t size>
class WS2812Chain{
protected:
    using Color = Color_t<real_t>;
    Gpio & gpio;
    std::array<WS2812Single, size> leds;

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
        __nopn(4);
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

public:
    WS2812Chain(Gpio & _gpio):gpio(_gpio){;}
    void init(){
        for(auto & led : leds) led = Color(0,0,0);
        gpio.OutPP();
    }

    WS2812Single & operator[](const int index){
        if(index < 0) return leds.at(size + index);
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

#endif
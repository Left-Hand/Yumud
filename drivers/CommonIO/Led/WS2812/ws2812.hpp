#pragma once

#include "../RgbLed.hpp"
#include "core/clock/clock.hpp"
#include <array>

namespace ymd::hal{
    class GpioIntf;
    
}
namespace ymd::drivers{


class WS2812_Phy{
public:
    WS2812_Phy(hal::GpioIntf & gpio):gpio_(gpio){;}

    __no_inline void delayLong();
    __no_inline void delayShort();

    void sendCode(const bool state);
    void sendByte(const uint8_t data);
    void sendReset();
    void init();
private:
    hal::GpioIntf & gpio_;
};


class WS2812: public RgbLedConcept{
protected:

    WS2812_Phy phy_;
    void _update(const Color &color);
public:
    WS2812(hal::GpioIntf & gpio):phy_(gpio){;}
    void init();
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
    WS2812_Phy phy_;
    std::array<WS2812Single, N> leds;

public:
    WS2812Chain(hal::GpioIntf & gpio):phy_(gpio){;}
    void init(){
        for(auto & led : leds) led = Color(0,0,0);
        phy_.init();
    }

    WS2812Single & operator[](const int index){
        if(index < 0) return leds.at(N + index);
        else return leds.at(index);
    }

    void refresh(){

        phy_.sendReset();

        for(auto & led : leds){
            uint16_t r,g,b;

            uni_to_u16(led.color.r, r);
            uni_to_u16(led.color.g, g);
            uni_to_u16(led.color.b, b);

            phy_.sendByte(g >> 8);
            phy_.sendByte(r >> 8);
            phy_.sendByte(b >> 8);

        }

    }
};

};

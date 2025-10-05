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

    __no_inline static void delay_long();
    __no_inline static void delay_short();

    void send_bit(const bool state);
    void send_byte(const uint8_t data);
    void send_reset();
    void init();
private:
    hal::GpioIntf & gpio_;
};


class WS2812: public RgbLedIntf{
public:
    explicit WS2812(hal::GpioIntf & gpio):phy_(gpio){;}
    void init();
    void set_rgb(const RGB<q16> &color);
private:
    WS2812_Phy phy_;
};

class WS2812Single: public RgbLedIntf{
public:
    RGB<q16> color;
    WS2812Single() = default;
};


template<size_t N>
class WS2812Chain{
protected:
    WS2812_Phy phy_;
    std::array<WS2812Single, N> leds;

public:
    WS2812Chain(hal::GpioIntf & gpio):phy_(gpio){;}
    void init(){
        for(auto & led : leds) led = RGB(0,0,0);
        phy_.init();
    }

    WS2812Single & operator[](const int index){
        if(index < 0) return leds.at(N + index);
        else return leds.at(index);
    }

    void refresh(){

        phy_.send_reset();

        for(auto & led : leds){
            uint16_t r,g,b;

            uni_to_u16(led.color.r, r);
            uni_to_u16(led.color.g, g);
            uni_to_u16(led.color.b, b);

            phy_.send_byte(CLAMP(uint8_t(r * 255), 0, 255));
            phy_.send_byte(CLAMP(uint8_t(g * 255), 0, 255));
            phy_.send_byte(CLAMP(uint8_t(b * 255), 0, 255));

        }

    }
};

};

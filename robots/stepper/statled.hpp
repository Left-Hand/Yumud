#ifndef __STEPPER_STATLED_HPP__

#define __STEPPER_STATLED_HPP__

#include "constants.hpp"
#include "../drivers/CommonIO/Led/rgbLed.hpp"
#include "../types/color/color_t.hpp"

using Sys::t;
using Color = Color_t<real_t>;

class StatLed{
public:
    enum class Method:uint8_t{
        Sine = 0,
        Saw,
        Squ,
        Tri
    };
protected:
    RgbLedConcept<true> & led;

    Color color_a;
    Color color_b;
    uint16_t period;

    Method method;
public:
    StatLed(RgbLedConcept<true> & _led) : led(_led){;}

    void init(){
        led.init();
    }

    void setPeriod(const uint16_t _period){
        period = _period;
    }

    void setTranstit(const Color & _color_a, const Color & _color_b, const Method _method){
        color_a = _color_a;
        color_b = _color_b;
        method = _method;
    }

    void run(){
        real_t ratio;
        real_t _t = t * real_t(100000 / period) * real_t(0.01);
        switch(method){
        case Method::Saw:
            ratio = frac(_t);
            break;
        case Method::Sine:
            ratio = abs(2 * frac(_t) - 1);
            break;
        case Method::Squ:
            ratio = sign(2 * frac(_t) - 1) * 0.5 + 0.5;
            break;
        case Method::Tri:
            ratio = abs(2 * frac(_t) - 1);
            break;
        }

        Color color_mux = color_a.linear_interpolate(color_b, ratio);
        led = color_mux;
    }
};

#endif
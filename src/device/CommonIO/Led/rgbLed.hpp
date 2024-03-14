#ifndef __RGB_HPP__

#define __RGB_HPP__

#include "Led.hpp"
#include "types/color/color_t.hpp¡°
#include "src/timer/pwm_channel.hpp"

template<bool com_anode>
class RgbLedInst{
protected:
    using Color = Color_t<real_t>;

    virtual void _update(const Color &color);
public:
    void update(const Color & color){
        if(com_anode) _update(color);
        else _update(color.invert());
    }
};

template<bool com_anode>
class RgbLedDigital:public RgbLedInst<com_anode>{
protected:
    GpioConcept & red_pin;
    GpioConcept & green_pin;
    GpioConcept & blue_pin;
    void _update(const Color &color) override{
        red_pin = (color.r > real_t(0.5));
        green_pin = (color.g > real_t(0.5));
        blue_pin = (color.b > real_t(0.5));
    }
public:
    RgbLedDigital(GpioConcept & _red_pin, GpioConcept & _green_pin, GpioConcept & _blue_pin):
            red_pin(_red_pin), green_pin(_green_pin), blue_pin(_blue_pin){;}
};

class RgbLedPwm:public RegLedInst<false>{
protected:
    PwmChannelConcept & red_channel;
    PwmChannelConcept & green_channel;
    PwmChannelConcept & blue_channel;
    void _update(const Color &color) override{
        red_channel = color.r;
        green_channel = color.g;
        blue_channel = color.b;
    }
public:
    RegLedPwm(PwmChannelConcept & _red_channel, PwmChannelConcept & _green_channel, PwmChannelConcept & _blue_channel):
            red_channel(_red_channel), green_channel(_green_channel), blue_channel(_blue_channel){;}
};


#endif


#ifndef __RGB_HPP__

#define __RGB_HPP__

#include "Led.hpp"
#include "../types/color/color_t.hpp"
#include "../hal/timer/pwm/pwm_channel.hpp"


class RgbLedConcept{
protected:
    using Color = Color_t<real_t>;
    real_t brightness = real_t(1);
    const bool com_anode = false;
    virtual void update(const Color &color) = 0;
public:

    virtual void init() = 0;

    void setBrightness(real_t _brightness){
        brightness = _brightness;
    }

    void setColor(const Color& color){
        if(com_anode) update(color * brightness);
        else update((color.inverted()) * brightness);
    }

    virtual RgbLedConcept & operator = (const Color & color) = 0;
};

class RgbLedDigital:public RgbLedConcept{
protected:
    using Color = Color_t<real_t>;
    GpioConcept & red_pin;
    GpioConcept & green_pin;
    GpioConcept & blue_pin;
    void update(const Color &color) override{
        red_pin = (color.r > real_t(0.5));
        green_pin = (color.g > real_t(0.5));
        blue_pin = (color.b > real_t(0.5));
    }
public:
    RgbLedDigital(GpioConcept & _red_pin, GpioConcept & _green_pin, GpioConcept & _blue_pin):
            red_pin(_red_pin), green_pin(_green_pin), blue_pin(_blue_pin){;}
    
    void init() override{
        red_pin.outpp();
        green_pin.outpp();
        blue_pin.outpp();
    }

    RgbLedDigital & operator = (const Color & color) override{update(color); return *this;}
};

template<bool com_anode>
class RgbLedPwm:public RgbLedConcept{
protected:
    using Color = Color_t<real_t>;
    PwmChannel & red_channel;
    PwmChannel & green_channel;
    PwmChannel & blue_channel;
    void update(const Color &color) override{
        red_channel = color.r / real_t(1);
        green_channel = color.g / real_t(1);
        blue_channel = color.b / real_t(1);
    }
public:
    RgbLedPwm(PwmChannel & _red_channel, PwmChannel & _green_channel, PwmChannel & _blue_channel):
            red_channel(_red_channel), green_channel(_green_channel), blue_channel(_blue_channel){;}
};



class RgbLedAnalog{
public:
    LedAnalog red;
    LedAnalog green;
    LedAnalog blue;

    // RgbLedAnalog()
};


#endif


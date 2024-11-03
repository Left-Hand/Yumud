#pragma once
#include "Led.hpp"
#include "types/color/color_t.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

class GpioConcept;

namespace yumud::drivers{
class RgbLedConcept{
protected:
    using Color = Color_t<real_t>;

    real_t brightness = real_t(1);
public:
    void setBrightness(real_t _brightness){
        brightness = _brightness;
    }

    virtual RgbLedConcept & operator = (const Color & color) = 0;
};

class RgbLedDigital:public RgbLedConcept{
protected:
    using Color = Color_t<real_t>;
    GpioConcept & red_gpio;
    GpioConcept & green_gpio;
    GpioConcept & blue_gpio;
public:
    RgbLedDigital(GpioConcept & _red_gpio, GpioConcept & _green_gpio, GpioConcept & _blue_gpio):
            red_gpio(_red_gpio), green_gpio(_green_gpio), blue_gpio(_blue_gpio){;}
    
    void init(){
        red_gpio.outpp();
        green_gpio.outpp();
        blue_gpio.outpp();
    }

    RgbLedDigital & operator = (const Color & color) override{
        red_gpio = (color.r > real_t(0.5));
        green_gpio = (color.g > real_t(0.5));
        blue_gpio = (color.b > real_t(0.5));
        return *this;
    }
};


class RgbLedAnalog:public RgbLedConcept{
public:
    LedAnalog red;
    LedAnalog green;
    LedAnalog blue;

    //template
    RgbLedAnalog(gpio_or_pwm auto & _red_ch, gpio_or_pwm auto & _green_ch, gpio_or_pwm auto & _blue_ch):
        red(_red_ch), green(_green_ch), blue(_blue_ch){;}

    // void init() override{
        // red.init();
        // green.init();
        // blue.init();
    // }

    RgbLedAnalog & operator = (const Color & color) override{
        red = color.r;    
        green = color.g;    
        blue = color.b;    
        return *this;
    }
};

};
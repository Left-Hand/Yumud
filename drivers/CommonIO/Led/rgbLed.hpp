#pragma once

#include "Led.hpp"
#include "types/colors/color/color.hpp"
#include "concept/pwm_channel.hpp"
#include "hal/gpio/gpio_intf.hpp"

class GpioIntf;

namespace ymd::drivers{
class RgbLedIntf{
protected:
    real_t brightness = real_t(1);
public:
    void setBrightness(real_t _brightness){
        brightness = _brightness;
    }

    virtual ~RgbLedIntf() = default;

    virtual RgbLedIntf & operator = (const Color<real_t> & color) = 0;
};

class RgbLedDigital:public RgbLedIntf{
protected:
    hal::GpioIntf & red_gpio_;
    hal::GpioIntf & green_gpio_;
    hal::GpioIntf & blue_gpio_;
public:
    RgbLedDigital(hal::GpioIntf & _red_gpio_, hal::GpioIntf & _green_gpio_, hal::GpioIntf & _blue_gpio_):
            red_gpio_(_red_gpio_), green_gpio_(_green_gpio_), blue_gpio_(_blue_gpio_){;}
    
    void init(){
        red_gpio_.outpp();
        green_gpio_.outpp();
        blue_gpio_.outpp();
    }

    RgbLedDigital & operator = (const Color<real_t> & color) override{
        red_gpio_ = BoolLevel::from(color.r > real_t(0.5));
        green_gpio_ = BoolLevel::from(color.g > real_t(0.5));
        blue_gpio_ = BoolLevel::from(color.b > real_t(0.5));
        return *this;
    }
};


class RgbLedAnalog:public RgbLedIntf{
public:
    LedAnalog red;
    LedAnalog green;
    LedAnalog blue;

    RgbLedAnalog(auto & _red_ch, auto & _green_ch, auto & _blue_ch):
        red(_red_ch), green(_green_ch), blue(_blue_ch){;}

    RgbLedAnalog & operator = (const Color<real_t> & color) override{
        red.set_dutycycle(color.r);    
        green.set_dutycycle(color.g);    
        blue.set_dutycycle(color.b);    
        return *this;
    }
};

};
#pragma once

#include "Led.hpp"
#include "primitive/colors/color/color.hpp"
#include "primitive/pwm_channel.hpp"
#include "hal/gpio/gpio_intf.hpp"

class GpioIntf;

namespace ymd::drivers{
class RgbLedIntf{
    virtual void set_rgb(const RGB<iq16> & rgb) = 0;
};

class DigitalRgbLed final{
public:
    DigitalRgbLed(
        hal::GpioIntf & _red_gpio_, 
        hal::GpioIntf & _green_gpio_, 
        hal::GpioIntf & _blue_gpio_
    ):
        red_gpio_(_red_gpio_), 
        green_gpio_(_green_gpio_), 
        blue_gpio_(_blue_gpio_){;}
    
    void init(){
        red_gpio_.outpp();
        green_gpio_.outpp();
        blue_gpio_.outpp();
    }

    void set_rgb(const RGB<iq16> & color){
        red_gpio_ = BoolLevel::from(color.r > iq16(0.5));
        green_gpio_ = BoolLevel::from(color.g > iq16(0.5));
        blue_gpio_ = BoolLevel::from(color.b > iq16(0.5));
    }
private:
    hal::GpioIntf & red_gpio_;
    hal::GpioIntf & green_gpio_;
    hal::GpioIntf & blue_gpio_;
};


class AnalogRgbLed final: public RgbLedIntf{
public:

    explicit AnalogRgbLed(
        auto & _red_ch, 
        auto & _green_ch, 
        auto & _blue_ch
    ):
        red_ch_(_red_ch), 
        green_ch_(_green_ch), 
        blue_ch_(_blue_ch){;}

    void set_rgb(const RGB<iq16> & color){
        red_ch_.set_dutycycle(color.r);    
        green_ch_.set_dutycycle(color.g);    
        blue_ch_.set_dutycycle(color.b);    
    }
private:
    hal::PwmIntf & red_ch_;
    hal::PwmIntf & green_ch_;
    hal::PwmIntf & blue_ch_;
};

};
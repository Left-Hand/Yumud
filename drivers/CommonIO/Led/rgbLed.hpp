#pragma once

#include "Led.hpp"
#include "primitive/colors/color/color.hpp"
#include "primitive/pwm_channel.hpp"
#include "hal/gpio/gpio.hpp"

class Gpio;

namespace ymd::drivers{
class RgbLedIntf{
    virtual void set_rgb(const RGB<iq16> & rgb) = 0;
};

class DigitalRgbLed final{
public:
    DigitalRgbLed(
        hal::Gpio & _red_pin_, 
        hal::Gpio & _green_pin_, 
        hal::Gpio & _blue_pin_
    ):
        red_pin_(_red_pin_), 
        green_pin_(_green_pin_), 
        blue_pin_(_blue_pin_){;}
    
    void init(){
        red_pin_.outpp();
        green_pin_.outpp();
        blue_pin_.outpp();
    }

    void set_rgb(const RGB<iq16> & color){
        red_pin_.write(BoolLevel::from(color.r > iq16(0.5)));
        green_pin_.write(BoolLevel::from(color.g > iq16(0.5)));
        blue_pin_.write(BoolLevel::from(color.b > iq16(0.5)));
    }
private:
    hal::Gpio & red_pin_;
    hal::Gpio & green_pin_;
    hal::Gpio & blue_pin_;
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
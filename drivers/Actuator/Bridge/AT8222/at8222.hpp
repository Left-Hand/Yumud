#pragma once

#include "../CoilDriver.hpp"

namespace ymd::drivers{

class AT8222:public Coil2DriverIntf{
protected:
    hal::PwmIntf & forward_pwm;
    hal::PwmIntf & backward_pwm;
    hal::GpioIntf * p_enable_gpio;

    AT8222(
        hal::PwmIntf & _forward_pwm, 
        hal::PwmIntf & _backward_pwm, 
        hal::GpioIntf * _p_en_gpio = nullptr):
    forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), p_enable_gpio(_p_en_gpio){;}
public:

    AT8222(
        hal::PwmIntf & _forward_pwm, 
        hal::PwmIntf & _backward_pwm, 
        hal::GpioIntf & _en_gpio):
    AT8222(_forward_pwm, _backward_pwm, &_en_gpio){;}

    AT8222(
        hal::PwmIntf & _forward_pwm, 
        hal::PwmIntf & _backward_pwm):
    AT8222(_forward_pwm, _backward_pwm, nullptr){;}


    void init();
    void enable(const bool en = true);

    __fast_inline AT8222 & operator = (const real_t duty);
};

AT8222 & AT8222::operator = (const real_t duty){
    if(duty > 0){
        forward_pwm = real_t(0);
        backward_pwm = MIN(duty, real_t(1));
    }else if(duty < 0){
        forward_pwm = MAX(-duty, real_t(-1));
        backward_pwm = real_t(0);
    }else{
        forward_pwm = real_t(0);
        backward_pwm = real_t(0);
    }
    return *this;
}

};
#pragma once

#include "../CoilDriver.hpp"
#include "hal/gpio/gpio_intf.hpp"

namespace ymd::drivers{

class AT8222:public Coil2DriverIntf{
protected:
    hal::PwmIntf & forward_pwm_;
    hal::PwmIntf & backward_pwm_;
    hal::GpioIntf & enable_gpio_;
public:

    AT8222(
        hal::PwmIntf & forward_pwm, 
        hal::PwmIntf & backward_pwm, 
        hal::GpioIntf & en_gpio):
    forward_pwm_(forward_pwm), backward_pwm_(backward_pwm), enable_gpio_(en_gpio){;}

    void init();
    void enable(const bool en = true);

    __fast_inline AT8222 & operator = (const real_t duty);
};

AT8222 & AT8222::operator = (const real_t duty){
    if(duty > 0){
        forward_pwm_ = real_t(0);
        backward_pwm_ = CLAMP(duty, 0, 1);
    }else if(duty < 0){
        forward_pwm_ = CLAMP(-duty, 0, 1);
        backward_pwm_ = real_t(0);
    }else{
        forward_pwm_ = real_t(0);
        backward_pwm_ = real_t(0);
    }
    return *this;
}

};
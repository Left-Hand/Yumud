#pragma once


#include "../CoilDriver.hpp"
#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"
#include "hal/gpio/gpio_intf.hpp"
namespace ymd::drivers{

class DRV8801:public Coil2DriverIntf{
protected:
    hal::PwmIntf & pwm_;
    hal::GpioIntf & dir_;
    hal::AnalogInIntf & curr_ch_;

    public:
    DRV8801(hal::PwmIntf & _pwm, hal::GpioIntf & _dir, hal::AnalogInIntf & _curr_ch):
        pwm_(_pwm),
        dir_(_dir),
        curr_ch_(_curr_ch){;}


    // void init();
    void enable(const Enable en){}


    __fast_inline DRV8801 & operator = (const real_t duty);
};

DRV8801 & DRV8801::operator = (const real_t duty){
    if(duty >= 0){
        pwm_ = MIN(duty, 1);
        dir_.set();
    }else{
        pwm_ = MAX(-duty, -1);
        dir_.clr();
    }
    return *this;
}

};
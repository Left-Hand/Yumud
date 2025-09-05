#pragma once


#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"
#include "hal/gpio/gpio_intf.hpp"

namespace ymd::drivers{

class DRV8801 final{
public:
    DRV8801(hal::PwmIntf & _pwm, hal::GpioIntf & _dir, hal::AnalogInIntf & _curr_ch):
        pwm_(_pwm),
        dir_(_dir),
        curr_ch_(_curr_ch){;}

    __fast_inline void set_dutycycle(const real_t duty){
        if(duty >= 0){
            pwm_.set_dutycycle(MIN(duty, 1));
            dir_.set();
        }else{
            pwm_.set_dutycycle(MAX(-duty, -1));
            dir_.clr();
        }
    }
private:
    hal::PwmIntf & pwm_;
    hal::GpioIntf & dir_;
    hal::AnalogInIntf & curr_ch_;
};
};
#pragma once

#include "pwm_channel.hpp"

#include <variant>

namespace ymd{

struct GpioConcept;

template<typename T>
concept gpio_or_pwm = std::is_base_of_v<GpioConcept, T> || std::is_base_of_v<PwmChannelIntf, T>;

class GpioOrPwm:public PwmChannelIntf{
public:
    using GpioRef = std::reference_wrapper<GpioConcept>; 
    using PwmRef = std::reference_wrapper<PwmChannelIntf>; 

    std::variant<GpioRef, PwmRef> inst_;

    GpioOrPwm(gpio_or_pwm auto & _inst):inst_(_inst){}


    // void init();

    bool hasPwm() const;
    bool hasGpio() const;

    PwmRef pwm();
    GpioRef gpio();
    GpioOrPwm & operator = (const real_t duty) override;
};

};
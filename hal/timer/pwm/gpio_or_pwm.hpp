#pragma once

#include "concept/pwm_channel.hpp"

#include <variant>

namespace ymd::hal{

struct GpioIntf;


template<typename T>
static constexpr bool is_gpio_v = std::is_base_of_v<GpioIntf, T>;

template<typename T>
static constexpr bool is_pwm_v = std::is_base_of_v<PwmIntf, T>;

template<typename T>
concept gpio_or_pwm = is_gpio_v<T> || is_pwm_v<T>;

class GpioOrPwm:public PwmIntf{
public:
    using GpioRef = std::reference_wrapper<GpioIntf>; 
    using PwmRef = std::reference_wrapper<PwmIntf>; 

    std::variant<GpioRef, PwmRef> inst_;

    GpioOrPwm(gpio_or_pwm auto & _inst):inst_(_inst){}


    // void init();

    bool hasPwm() const;
    bool hasGpio() const;

    PwmRef pwm();
    GpioRef gpio();
    void set_dutycycle(const real_t duty) override;
};

};
#pragma once

#include "../CoilDriver.hpp"

using OptionlGpio = std::optional<GpioConcept &>;

class DRV8313:public Coil3Driver{
protected:
    using PWM3 = std::array<PwmChannel &, 3>;
    using EnablePort = PortVirtual<3>;

    PWM3 pwms_;
    EnablePort en_gpios_;
    OptionlGpio slp_gpio_;
public:
    DRV8313(PWM3 && _pwms, EnablePort && _en_port, OptionlGpio&& _slp_gpio):
        pwms_(std::move(_pwms)),
        en_gpios_(std::move(_en_port)),
        slp_gpio_(std::move(_slp_gpio))
    {}

    void sleep(const bool en = true){
        if(slp_gpio_.has_value()) (*slp_gpio_) = en;
    }

    void enable(const bool en = true){
        for(auto & gpio : en_gpios_){
            if(gpio.isValid()) gpio = en;
        }
    }

    DRV8313 & operator=(const UVW_Duty & duty){
        auto [u, v, w] = duty;

        pwms_[0] = u;
        pwms_[1] = v;
        pwms_[2] = w;
    }
};
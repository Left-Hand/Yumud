#include "gpio_or_pwm.hpp"

#include "hal/gpio/port_virtual.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

void GpioOrPwm::init(){
    if(std::holds_alternative<GpioRef>(inst_)){
        std::get<GpioRef>(inst_).get().outpp();
    }else if(std::holds_alternative<PwmRef>(inst_)){
        std::get<PwmRef>(inst_).get().init();
    };
}

bool GpioOrPwm::hasPwm() const {
    return std::holds_alternative<PwmRef>(inst_);
}
bool GpioOrPwm::hasGpio() const {
    return std::holds_alternative<GpioRef>(inst_);
}

GpioOrPwm & GpioOrPwm::operator = (const real_t duty){
    if(std::holds_alternative<GpioRef>(inst_)){
        std::get<GpioRef>(inst_).get() = (duty > real_t(0.5));
    }else if(std::holds_alternative<PwmRef>(inst_)){
        std::get<PwmRef>(inst_).get() = duty;
    };

    return *this;
}

GpioOrPwm::GpioRef GpioOrPwm::gpio(){
    return std::get<GpioRef>(inst_);
}

GpioOrPwm::PwmRef GpioOrPwm::pwm(){
    return std::get<PwmRef>(inst_);
}
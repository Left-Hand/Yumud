#include "gpio_or_pwm.hpp"

#include "hal/gpio/vport.hpp"
#include "concept/pwm_channel.hpp"

using namespace ymd::hal;


bool GpioOrPwm::hasPwm() const {
    return std::holds_alternative<PwmRef>(inst_);
}
bool GpioOrPwm::hasGpio() const {
    return std::holds_alternative<GpioRef>(inst_);
}
void GpioOrPwm::set_dutycycle(const real_t dutycycle){
    if(std::holds_alternative<GpioRef>(inst_)){
        std::get<GpioRef>(inst_).get() = BoolLevel::from(dutycycle > real_t(0.5));
    }else if(std::holds_alternative<PwmRef>(inst_)){
        std::get<PwmRef>(inst_).get().set_dutycycle(dutycycle);
    };
}

GpioOrPwm::GpioRef GpioOrPwm::gpio(){
    return std::get<GpioRef>(inst_);
}

GpioOrPwm::PwmRef GpioOrPwm::pwm(){
    return std::get<PwmRef>(inst_);
}
#include "Led.hpp"

#include "hal/gpio/port_virtual.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

LedGpio &  LedGpio::operator =(const real_t duty){
    inst = (duty > 0.5) ^ inversed;
    return *this;
}

void LedGpio::toggle() {
    state =!state;
    inst = state ^ inversed;
}


// void LedPwm::set_duty(const real_t duty){
//     inst = inversed ? 1 - duty : duty;
// }


// void LedPwm::toggle() {
//     val = 1 - val;
//     inst = val;
// }

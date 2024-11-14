#include "Led.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

using namespace ymd;
using namespace ymd::drivers;

LedGpio &  LedGpio::operator =(const real_t duty){
    inst = (duty > real_t(0.5)) ^ inversed;
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

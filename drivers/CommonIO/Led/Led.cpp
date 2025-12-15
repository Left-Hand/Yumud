#include "Led.hpp"

#include "hal/gpio/gpio.hpp"
#include "primitive/pwm_channel.hpp"

using namespace ymd;
using namespace ymd::drivers;

// LedGpio &  LedGpio::operator =(const real_t dutycycle{
//     inst_ = BoolLevel::from((duty > real_t(0.5)) ^ inversed);
//     return *this;
// }

void LedGpio::set_dutycycle(const real_t dutycycle){
    inst_.write(BoolLevel::from((dutycycle > real_t(0.5)) ^ inversed));
}


void LedGpio::toggle() {
    state =!state;
    inst_.write(BoolLevel::from(state ^ inversed));
}


// void LedPwm::set_dutycycle(const real_t dutycycle{
//     inst = inversed ? 1 - duty : duty;
// }


// void LedPwm::toggle() {
//     val = 1 - val;
//     inst = val;
// }

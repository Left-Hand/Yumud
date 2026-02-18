#include "Led.hpp"

#include "hal/gpio/gpio.hpp"
#include "primitive/pwm_channel.hpp"

using namespace ymd;
using namespace ymd::drivers;

// LedGpio &  LedGpio::operator =(const iq16 dutycycle{
//     inst_ = BoolLevel::from((duty > iq16(0.5)) ^ inversed);
//     return *this;
// }

void LedGpio::set_dutycycle(const iq16 dutycycle){
    inst_.write(BoolLevel::from((dutycycle > iq16(0.5)) ^ inversed));
}


void LedGpio::toggle() {
    state =!state;
    inst_.write(BoolLevel::from(state ^ inversed));
}


// void LedPwm::set_dutycycle(const iq16 dutycycle{
//     inst = inversed ? 1 - duty : duty;
// }


// void LedPwm::toggle() {
//     val = 1 - val;
//     inst = val;
// }

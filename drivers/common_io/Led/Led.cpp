#include "Led.hpp"

#include "hal/gpio/gpio.hpp"
#include "primitive/pwm_channel.hpp"

using namespace ymd;
using namespace ymd::drivers;

void LedGpio::set_dutycycle(const iq16 dutycycle){
    inst_.write(BoolLevel::from((dutycycle > iq16(0.5)) ^ inversed));
}


void LedGpio::toggle() {
    state =!state;
    inst_.write(BoolLevel::from(state ^ inversed));
}


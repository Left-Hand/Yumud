#include "buzzer.hpp"

#include "hal/timer/pwm/pwm_channel.hpp"

namespace gxm{

Buzzer & Buzzer::operator =(const bool en){
    instance_ = en ? real_t(0.5) : real_t(0);
    return *this;    
}

}
#include "buzzer.hpp"

#include "concept/pwm_channel.hpp"


using namespace ymd;
using namespace gxm;

Buzzer & Buzzer::operator =(const bool en){
    instance_ = en ? real_t(0.5) : real_t(0);
    return *this;    
}

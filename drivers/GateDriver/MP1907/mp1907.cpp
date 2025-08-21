#include "mp1907.hpp"

#include "hal/timer/timer_oc.hpp"
#include "hal/timer/timer.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

// MP1907::MP1907(AdvancedTimer & timer, const uint8_t pair_index, hal::GpioIntf & en_gpio):
//     inst_(timer.oc(pair_index)),
//     inst_n_(timer.ocn(pair_index)),
//     en_gpio_(en_gpio){;}

void MP1907::init(){
    inst_.init({});
    inst_n_.init({});
}

MP1907 & MP1907::operator=(const real_t duty){
    inst_.set_dutycycle(duty_range_.clamp(duty));
    return *this;
}
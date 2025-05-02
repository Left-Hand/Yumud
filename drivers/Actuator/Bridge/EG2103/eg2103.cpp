#include "eg2103.hpp"

#include "hal/timer/timer_oc.hpp"

using namespace ymd::drivers;
using namespace ymd::hal;

void EG2103::init(){
    inst_.init({});

    if(p_inst_n_){
        p_inst_n_->init({});
    }
}

EG2103 & EG2103::operator=(const real_t duty){
    inst_ = duty_range.clamp(duty);
    return *this;
}
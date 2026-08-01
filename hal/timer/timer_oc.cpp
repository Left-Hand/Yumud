#include "timer_oc.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, TIM_TypeDef)>(x))
// #define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))



void TimerOutBase::set_valid_level(const BoolLevel level){
    lld::timeroc_set_valid_level(p_inst_, sel_, level);
}

TimerOC::TimerOC(void * inst, const ChannelSelection sel):
    TimerOutBase(inst, sel), 
        cvr_(from_channel_to_cvr(inst, sel)), 
        arr_(SPL_INST(p_inst_)->ATRLR){;}
        
void TimerOC::init(const TimerOcPwmConfig & cfg){
    set_oc_mode(cfg.oc_mode);
    enable_cvr_sync(cfg.cvr_sync_en);
    set_valid_level(cfg.valid_level);
    enable_output(cfg.out_en);
}

void TimerOCN::init(const TimerOcnPwmConfig & cfg){
    enable_output(cfg.out_en);
}

void TimerOC::set_oc_mode(const TimerOC::Mode mode){
    lld::timeroc_set_oc_mode(p_inst_, sel_, mode);
}


void TimerOutBase::enable_output(const Enable en){
    lld::timeroc_enable_output(p_inst_, sel_, en);
}

void TimerOC::enable_cvr_sync(const Enable en){
    lld::timeroc_enable_cvr_sync(p_inst_, sel_, en);
}

// Gpio TimerOC::io(){
//     return timer::details::get_pin(SPL_INST(p_inst_), remap, sel_);
// }

// Gpio TimerOCN::io(){
//     return timer::details::get_pin(SPL_INST(p_inst_), remap, sel_);
// }

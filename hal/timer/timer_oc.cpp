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


namespace{

static void timeroc_set_oc_mode(
    void * p_inst,
    const TimerOC::ChannelSelection ch_sel, 
    const TimerOC::Mode mode
){
    using ChannelSelection = TimerOC::ChannelSelection;
    const uint8_t bits = std::bit_cast<uint8_t>(mode) << 4;

    switch(ch_sel.kind()){
        case ChannelSelection::CH1:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR1;
            const uint16_t m_code = TIM_OC1M;
            const uint16_t s_code = TIM_CC1S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(bits);
            SPL_INST(p_inst)->CHCTLR1 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH2:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR1;
            const uint16_t m_code = TIM_OC2M;
            const uint16_t s_code = TIM_CC2S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(bits) << 8);
            SPL_INST(p_inst)->CHCTLR1 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH3:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR2;
            const uint16_t m_code = TIM_OC3M;
            const uint16_t s_code = TIM_CC3S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(bits);
            SPL_INST(p_inst)->CHCTLR2 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH4:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR2;
            const uint16_t m_code = TIM_OC4M;
            const uint16_t s_code = TIM_CC4S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code << 8)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(bits) << 8);
            SPL_INST(p_inst)->CHCTLR2 = tmpccmrx;
            break;
        }
        default:
            __builtin_trap();
            break;
    }
}



static void timeroc_enable_output(
    void * p_inst,
    const TimerOC::ChannelSelection ch_sel,
    const Enable en
){
    if(en == EN) SPL_INST(p_inst)->CCER |= (1 << (std::bit_cast<uint8_t>(ch_sel) * 2));
    else SPL_INST(p_inst)->CCER &= (~(1 << ((std::bit_cast<uint8_t>(ch_sel)) * 2)));
}



static void timeroc_enable_cvr_sync(
    void * p_inst,
    const TimerOC::ChannelSelection ch_sel,
    const Enable en
){
    using ChannelSelection = TimerOC::ChannelSelection;
    const auto e_code = (en == EN) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable;
    switch(ch_sel.kind()){
        case ChannelSelection::CH1:
            TIM_OC1PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        case ChannelSelection::CH2:
            TIM_OC2PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        case ChannelSelection::CH3:
            TIM_OC3PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        case ChannelSelection::CH4:
            TIM_OC4PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        default:
            __builtin_trap();
            break;
    }
}

static void timeroc_set_valid_level(
    void * p_inst,
    const TimerOC::ChannelSelection ch_sel,
    const BoolLevel level
){
    if(level == LOW) SPL_INST(p_inst)->CCER |= (1 << (std::bit_cast<uint8_t>(ch_sel) * 2 + 1));
    else SPL_INST(p_inst)->CCER &= (~(1 << ((std::bit_cast<uint8_t>(ch_sel)) * 2 + 1)));
}
}


void TimerOutBase::set_valid_level(const BoolLevel level){
    timeroc_set_valid_level(p_inst_, sel_, level);
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
    timeroc_set_oc_mode(p_inst_, sel_, mode);
}


void TimerOutBase::enable_output(const Enable en){
    timeroc_enable_output(p_inst_, sel_, en);
}

void TimerOC::enable_cvr_sync(const Enable en){
    timeroc_enable_cvr_sync(p_inst_, sel_, en);
}

// Gpio TimerOC::io(){
//     return timer::details::get_pin(SPL_INST(p_inst_), remap, sel_);
// }

// Gpio TimerOCN::io(){
//     return timer::details::get_pin(SPL_INST(p_inst_), remap, sel_);
// }

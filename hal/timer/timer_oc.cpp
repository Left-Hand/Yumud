#include "timer_oc.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, TIM_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


// void TimerOutBase::plant_to_pin(const Enable en, const TimerRemap remap){
//     auto gpio = timer::details::get_pin(SDK_INST(inst_), remap, sel_);
//     if(en == EN) gpio.afpp();
// }


void TimerOutBase::set_valid_level(const BoolLevel level){
    if(level == LOW) SDK_INST(inst_)->CCER |= (1 << (std::bit_cast<uint8_t>(sel_) * 2 + 1));
    else SDK_INST(inst_)->CCER &= (~(1 << ((std::bit_cast<uint8_t>(sel_)) * 2 + 1)));
}

TimerOC::TimerOC(void * inst, const ChannelSelection sel):
    TimerOutBase(inst, sel), 
        cvr_(from_channel_to_cvr(inst, sel)), 
        arr_(SDK_INST(inst_)->ATRLR){;}
        
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

    const uint8_t bits = std::bit_cast<uint8_t>(mode) << 4;

    switch(sel_.kind()){
        default: __builtin_trap();
        case ChannelSelection::CH1:{
            uint16_t tmpccmrx = SDK_INST(inst_)->CHCTLR1;
            const uint16_t m_code = TIM_OC1M;
            const uint16_t s_code = TIM_CC1S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(bits);
            SDK_INST(inst_)->CHCTLR1 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH2:{
            uint16_t tmpccmrx = SDK_INST(inst_)->CHCTLR1;
            const uint16_t m_code = TIM_OC2M;
            const uint16_t s_code = TIM_CC2S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(bits) << 8);
            SDK_INST(inst_)->CHCTLR1 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH3:{
            uint16_t tmpccmrx = SDK_INST(inst_)->CHCTLR2;
            const uint16_t m_code = TIM_OC3M;
            const uint16_t s_code = TIM_CC3S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(bits);
            SDK_INST(inst_)->CHCTLR2 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH4:{
            uint16_t tmpccmrx = SDK_INST(inst_)->CHCTLR2;
            const uint16_t m_code = TIM_OC4M;
            const uint16_t s_code = TIM_CC4S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code << 8)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(bits) << 8);
            SDK_INST(inst_)->CHCTLR2 = tmpccmrx;
            break;
        }
    }

}
void TimerOutBase::enable_output(const Enable en){
    if(en == EN) SDK_INST(inst_)->CCER |= (1 << (std::bit_cast<uint8_t>(sel_) * 2));
    else SDK_INST(inst_)->CCER &= (~(1 << ((std::bit_cast<uint8_t>(sel_)) * 2)));
}

void TimerOC::enable_cvr_sync(const Enable en){
    const auto e_code = (en == EN) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable;
    switch(sel_.kind()){
        case ChannelSelection::CH1:
            TIM_OC1PreloadConfig(SDK_INST(inst_), e_code);
            break;
        case ChannelSelection::CH2:
            TIM_OC2PreloadConfig(SDK_INST(inst_), e_code);
            break;
        case ChannelSelection::CH3:
            TIM_OC3PreloadConfig(SDK_INST(inst_), e_code);
            break;
        case ChannelSelection::CH4:
            TIM_OC4PreloadConfig(SDK_INST(inst_), e_code);
            break;
        default:
            __builtin_trap();
            break;
    }
}

// Gpio TimerOC::io(){
//     return timer::details::get_pin(SDK_INST(inst_), remap, sel_);
// }

// Gpio TimerOCN::io(){
//     return timer::details::get_pin(SDK_INST(inst_), remap, sel_);
// }

#include "timer_oc.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "core/system.hpp"

using namespace ymd;
using namespace ymd::hal;


void TimerOutBase::install_to_pin(const Enable en){
    auto gpio = details::get_pin(inst_, nth_);
    if(en == EN) gpio.afpp();
}


void TimerOutBase::set_valid_level(const BoolLevel level){
    if(level == LOW) inst_->CCER |= (1 << (std::bit_cast<uint8_t>(nth_) * 2 + 1));
    else inst_->CCER &= (~(1 << ((std::bit_cast<uint8_t>(nth_)) * 2 + 1)));
}

void TimerOC::init(const TimerOcPwmConfig & cfg){
    set_oc_mode(cfg.oc_mode);
    enable_cvr_sync(cfg.cvr_sync_en);
    set_valid_level(cfg.valid_level);
    install_to_pin(cfg.install_en);
    enable_output(cfg.out_en);
}

void TimerOCN::init(const TimerOcnPwmConfig & cfg){
    install_to_pin(cfg.install_en);
    enable_output(cfg.out_en);
}

void TimerOC::set_oc_mode(const TimerOC::Mode mode){
    using enum ChannelNth;

    const uint8_t raw_code = uint8_t(mode) << 4;

    switch(nth_){
        default: ymd::sys::abort();
        case CH1:{
            uint16_t tmpccmrx = inst_->CHCTLR1;
            const uint16_t m_code = TIM_OC1M;
            const uint16_t s_code = TIM_CC1S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(raw_code);
            inst_->CHCTLR1 = tmpccmrx;
            break;
        }
        case CH2:{
            uint16_t tmpccmrx = inst_->CHCTLR1;
            const uint16_t m_code = TIM_OC2M;
            const uint16_t s_code = TIM_CC2S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(raw_code) << 8);
            inst_->CHCTLR1 = tmpccmrx;
            break;
        }
        case CH3:{
            uint16_t tmpccmrx = inst_->CHCTLR2;
            const uint16_t m_code = TIM_OC3M;
            const uint16_t s_code = TIM_CC3S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(raw_code);
            inst_->CHCTLR2 = tmpccmrx;
            break;
        }
        case CH4:{
            uint16_t tmpccmrx = inst_->CHCTLR2;
            const uint16_t m_code = TIM_OC4M;
            const uint16_t s_code = TIM_CC4S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code << 8)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(raw_code) << 8);
            inst_->CHCTLR2 = tmpccmrx;
            break;
        }
    }

}
void TimerOutBase::enable_output(const Enable en){
    if(en == EN) inst_->CCER |= (1 << (std::bit_cast<uint8_t>(nth_) * 2));
    else inst_->CCER &= (~(1 << ((std::bit_cast<uint8_t>(nth_)) * 2)));
}

void TimerOC::enable_cvr_sync(const Enable en){
    using enum ChannelNth;

    switch(nth_){
        case CH1:
            TIM_OC1PreloadConfig(inst_, (en == EN) ? 
                TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH2:
            TIM_OC2PreloadConfig(inst_, (en == EN) ? 
                TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH3:
            TIM_OC3PreloadConfig(inst_, (en == EN) ? 
                TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH4:
            TIM_OC4PreloadConfig(inst_, (en == EN) ? 
                TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        default:
            sys::abort();
            break;
    }
}

Gpio TimerOC::io(){
    return details::get_pin(inst_, nth_);
}

Gpio TimerOCN::io(){
    return details::get_pin(inst_, nth_);
}

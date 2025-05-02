#include "timer_oc.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;


void TimerOut::install_to_pin(const Enable en){
    Gpio & io = internal::get_pin(instance, idx_);
    if(en)io.afpp();
    else io.inflt();
}


void TimerOut::set_valid_level(const BoolLevel level){
    if(level == LOW) instance->CCER |= (1 << ((uint8_t)idx_ * 2 + 1));
    else instance->CCER &= (~(1 << (((uint8_t)idx_) * 2 + 1)));
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
    using enum ChannelIndex;

    uint16_t m_code;
    uint16_t s_code;
    // volatile uint16_t *ctlr_reg;
    switch(idx_){
        default:
            HALT;
            [[fallthrough]];

        case CH1:
            {
                uint16_t tmpccmrx = instance->CHCTLR1;
                m_code = TIM_OC1M;
                s_code = TIM_CC1S;
                tmpccmrx &= (uint16_t)(~((uint16_t)(m_code)));
                tmpccmrx &= (uint16_t)(~((uint16_t)(s_code)));
                tmpccmrx |= (uint16_t)mode;
                instance->CHCTLR1 = tmpccmrx;
                break;
            }
        case CH2:
            {
                uint16_t tmpccmrx = instance->CHCTLR1;
                m_code = TIM_OC2M;
                s_code = TIM_CC2S;
                tmpccmrx &= (uint16_t)(~((uint16_t)(m_code)));
                tmpccmrx &= (uint16_t)(~((uint16_t)(s_code)));
                tmpccmrx |= (uint16_t)((uint16_t)mode << 8);
                instance->CHCTLR1 = tmpccmrx;
                break;
            }
        case CH3:
            {
                uint16_t tmpccmrx = instance->CHCTLR2;
                m_code = TIM_OC3M;
                s_code = TIM_CC3S;
                tmpccmrx &= (uint16_t)(~((uint16_t)(m_code)));
                tmpccmrx &= (uint16_t)(~((uint16_t)(s_code)));
                tmpccmrx |= (uint16_t)mode;
                instance->CHCTLR2 = tmpccmrx;
                break;
            }
        case CH4:
            {
                uint16_t tmpccmrx = instance->CHCTLR2;
                m_code = TIM_OC4M;
                s_code = TIM_CC4S;
                tmpccmrx &= (uint16_t)(~((uint16_t)(m_code << 8)));
                tmpccmrx &= (uint16_t)(~((uint16_t)(s_code)));
                tmpccmrx |= (uint16_t)((uint16_t)mode << 8);
                instance->CHCTLR2 = tmpccmrx;
                break;
            }
    }

}
void TimerOut::enable_output(const Enable en){
    if(en) instance->CCER |= (1 << ((uint8_t)idx_ * 2));
    else instance->CCER &= (~(1 << (((uint8_t)idx_) * 2)));
}

void TimerOut::enable_cvr_sync(const Enable en){
    using enum ChannelIndex;

    switch(idx_){
        case CH1:
            TIM_OC1PreloadConfig(instance, (en == EN) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH2:
            TIM_OC2PreloadConfig(instance, (en == EN) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH3:
            TIM_OC3PreloadConfig(instance, (en == EN) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH4:
            TIM_OC4PreloadConfig(instance, (en == EN) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        default:
            break;
    }
}

Gpio & TimerOC::io(){
    return internal::get_pin(instance, idx_);
}

Gpio & TimerOCN::io(){
    return internal::get_pin(instance, idx_);
}

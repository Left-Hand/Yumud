#include "timer_oc.hpp"
#include "hal/gpio/port.hpp"

using namespace ymd;
using namespace ymd::TimerUtils;
using namespace ymd::TimerUtils::internal;



TimerOC & TimerOC::init(const TimerOC::Mode mode, const bool install){
    // sync();

    setMode(mode);

    if(install){
        installToPin();
    }

    enable();

    return *this;
}

TimerOC & TimerOC::setMode(const TimerOC::Mode mode){
    using enum ChannelIndex;

    uint16_t m_code;
    uint16_t s_code;
    // volatile uint16_t *ctlr_reg;
    switch(idx_){
        default:
            HALT;
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

    return *this;
}

void TimerOut::installToPin(const bool en){
    Gpio & io = getPin(instance, idx_);
    if(en)io.afpp();
    else io.inflt();
    enable(en);
}

TimerOut & TimerOut::enable(const bool en){
    if(en) instance->CCER |= 1 << ((uint8_t)idx_ * 2);
    else instance->CCER &= ~(1 << ((uint8_t)idx_) * 2);

    return *this;
}


TimerOut & TimerOut::setPolarity(const bool pol){
    if(!pol) instance->CCER |= (1 << ((uint8_t)idx_ * 2 + 1));
    else instance->CCER &= (~(1 << (((uint8_t)idx_) * 2 + 1)));

    return *this;
}

TimerOut & TimerOut::setOutputState(const bool s){
    if(s) instance->CCER |= (1 << ((uint8_t)idx_ * 2));
    else instance->CCER &= (~(1 << (((uint8_t)idx_) * 2)));

    return *this;
}

TimerOut & TimerOut::sync(const bool _sync){
    using enum ChannelIndex;

    switch(idx_){
        case CH1:
            TIM_OC1PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH2:
            TIM_OC2PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH3:
            TIM_OC3PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case CH4:
            TIM_OC4PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        default:
            break;
    }

    return *this;
}


TimerOut & TimerOut::setIdleState(const bool state){
    if(isAdvancedTimer(instance)){
        auto tmpcr2 = instance->CTLR2;
        const auto mask = (uint16_t)(TIM_OIS1 << (uint8_t)idx_);
        tmpcr2 &= (uint16_t)(~mask);
        if(state) tmpcr2 |= mask;
        instance->CTLR2 = tmpcr2;
    }

    return *this;
}

Gpio & TimerOC::io(){
    return getPin(instance, idx_);
}

Gpio & TimerOCN::io(){
    return getPin(instance, idx_);
}

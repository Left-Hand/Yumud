#include "timer_oc.hpp"
#include "hal/gpio/port.hpp"



void TimerOut::installToPin(const bool & en){
    Gpio & io = TimerUtils::getPin(instance, channel);
    if(en)io.afpp();
    else io.inflt();
    enable(en);
}

void TimerOC::init(const TimerOC::Mode mode, const bool install){
    setPolarity(true);
    enableSync();

    setMode(mode);

    if(install){
        installToPin();
    }

    enable();
}

void TimerOC::setMode(const TimerOC::Mode mode){
    uint16_t m_code,s_code;
    switch(channel){
        default:
            return;
        case Channel::CH1:
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
        case Channel::CH2:
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
        case Channel::CH3:
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
        case Channel::CH4:
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

void TimerOut::enable(const bool & en){
    if(en) instance->CCER |= 1 << ((uint8_t)channel * 2);
    else instance->CCER &= ~(1 << ((uint8_t)channel) * 2);
}


void TimerOut::setPolarity(const bool & pol){
    if(!pol) instance->CCER |= (1 << ((uint8_t)channel * 2 + 1));
    else instance->CCER &= (~(1 << (((uint8_t)channel) * 2 + 1)));
}

void TimerOut::enableSync(const bool & _sync){
    switch(channel){
        case Channel::CH1:
            TIM_OC1PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case Channel::CH2:
            TIM_OC2PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case Channel::CH3:
            TIM_OC3PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        case Channel::CH4:
            TIM_OC4PreloadConfig(instance, _sync ? TIM_OCPreload_Enable : TIM_OCPreload_Disable);
            break;
        default:
            break;
    }
}

void TimerOut::setIdleState(const bool & state){
    if(TimerUtils::isAdvancedTimer(instance)){
        auto tmpcr2 = instance->CTLR2;
        const auto mask = (uint16_t)(TIM_OIS1 << (uint8_t)channel);
        tmpcr2 &= (uint16_t)(~mask);
        if(state) tmpcr2 |= mask;
        instance->CTLR2 = tmpcr2;
    }
}
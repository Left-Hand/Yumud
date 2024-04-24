#include "timer_oc.hpp"
#include "src/gpio/port.hpp"

volatile uint16_t & TimerOutChannelOnChip::from_channel_to_cvr(const Channel & _channel){
    switch(_channel){
        default:
        case Channel::CH1:
        case Channel::CH1N:
            return (instance->CH1CVR);
        case Channel::CH2:
        case Channel::CH2N:
            return (instance->CH2CVR);
        case Channel::CH3:
        case Channel::CH3N:
            return (instance->CH3CVR);
        case Channel::CH4:
            return (instance->CH4CVR);
    }
}


TimerOutChannelPosOnChip::TimerOutChannelPosOnChip(TIM_TypeDef * _base, const Channel & _channel):
        TimerOutChannelOnChip(_base, _channel),
        cvr(from_channel_to_cvr(_channel)),
        arr(_base->ATRLR){

    setPolarity(true);
    enableSync(false);
}

void TimerOutChannelOnChip::init(const bool & install, const Mode & mode){
    setMode(mode);

    if(install){
        installToPin();
    }

    enable();
}

void TimerOutChannelOnChip::setMode(const Mode & mode){
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

void TimerOutChannelOnChip::enable(const bool & en){
    if(en) instance->CCER |= 1 << ((uint8_t)channel * 2);
    else instance->CCER &= ~(1 << ((uint8_t)channel) * 2);
}


void TimerOutChannelOnChip::setPolarity(const bool & pol){
    if(!pol) instance->CCER |= (1 << ((uint8_t)channel * 2 + 1));
    else instance->CCER &= (~(1 << (((uint8_t)channel) * 2 + 1)));
}

void TimerOutChannelOnChip::enableSync(const bool & _sync){
    switch(channel){
        case Channel::CH1:
            TIM_OC1PreloadConfig(instance, _sync ? TIM_OC1PE : (uint16_t)0);
            break;
        case Channel::CH2:
            TIM_OC2PreloadConfig(instance, _sync ? TIM_OC2PE : (uint16_t)0);
            break;
        case Channel::CH3:
            TIM_OC3PreloadConfig(instance, _sync ? TIM_OC3PE : (uint16_t)0);
            break;
        case Channel::CH4:
            TIM_OC4PreloadConfig(instance, _sync ? TIM_OC4PE : (uint16_t)0);
            break;
        default:
            break;
    }

}

void TimerOutChannelOnChip::setIdleState(const bool & state){
    // TIM_OC1Init();
    if(isAdvancedTimer(instance)){
        auto tmpcr2 = instance->CTLR2;
        const auto mask = (uint16_t)(TIM_OIS1 << (uint8_t)channel);
        tmpcr2 &= (uint16_t)(~mask);
        if(state) tmpcr2 |= mask;
        instance->CTLR2 = tmpcr2;
    }
}


#define ADVANCED_TIMER_GPIO_TEMPLATE(x)\
case TIM##x##_BASE:\
    switch(channel){\
        default:\
        case Channel::CH1:\
            gpio_port = &TIM##x##_CH1_Port;\
            gpio_pin = TIM##x##_CH1_Pin;\
            break;\
        case Channel::CH1N:\
            gpio_port = &TIM##x##_CH1N_Port;\
            gpio_pin = TIM##x##_CH1N_Pin;\
            break;\
        case Channel::CH2:\
            gpio_port = &TIM##x##_CH2_Port;\
            gpio_pin = TIM##x##_CH2_Pin;\
            break;\
        case Channel::CH2N:\
            gpio_port = &TIM##x##_CH2N_Port;\
            gpio_pin = TIM##x##_CH2N_Pin;\
            break;\
        case Channel::CH3:\
            gpio_port = &TIM##x##_CH3_Port;\
            gpio_pin = TIM##x##_CH3_Pin;\
            break;\
        case Channel::CH3N:\
            gpio_port = &TIM##x##_CH3N_Port;\
            gpio_pin = TIM##x##_CH3N_Pin;\
            break;\
        case Channel::CH4:\
            gpio_port = &TIM##x##_CH4_Port;\
            gpio_pin = TIM##x##_CH4_Pin;\
            break;\
    }\
    break;\

#define GENERIC_TIMER_GPIO_TEMPLATE(x)\
case TIM##x##_BASE:\
    switch(channel){\
        default:\
        case Channel::CH1:\
            gpio_port = &TIM##x##_CH1_Port;\
            gpio_pin = TIM##x##_CH1_Pin;\
            break;\
        case Channel::CH2:\
            gpio_port = &TIM##x##_CH2_Port;\
            gpio_pin = TIM##x##_CH2_Pin;\
            break;\
        case Channel::CH3:\
            gpio_port = &TIM##x##_CH3_Port;\
            gpio_pin = TIM##x##_CH3_Pin;\
            break;\
        case Channel::CH4:\
            gpio_port = &TIM##x##_CH4_Port;\
            gpio_pin = TIM##x##_CH4_Pin;\
            break;\
    }\
    break;\

void TimerOutChannelOnChip::installToPin(const bool & en){
    Port * gpio_port;
    uint16_t gpio_pin = 0;
    switch((uint32_t)instance){
        default:
        #ifdef HAVE_TIM1
        ADVANCED_TIMER_GPIO_TEMPLATE(1)
        #endif

        #ifdef HAVE_TIM2
        GENERIC_TIMER_GPIO_TEMPLATE(2)
        #endif

        #ifdef HAVE_TIM3
        GENERIC_TIMER_GPIO_TEMPLATE(3)
        #endif

        #ifdef HAVE_TIM4
        GENERIC_TIMER_GPIO_TEMPLATE(4)
        #endif

        #ifdef HAVE_TIM5
        GENERIC_TIMER_GPIO_TEMPLATE(5)
        #endif

        #ifdef HAVE_TIM8
        ADVANCED_TIMER_GPIO_TEMPLATE(8)
        #endif
    }

    Gpio & io = (*gpio_port)[(Pin)gpio_pin];
    if(en)io.OutAfPP();
    else io.InFloating();
    enable(en);
}
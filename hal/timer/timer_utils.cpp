#include "timer_utils.hpp"

bool TimerUtils::isBasicTimer(const TIM_TypeDef * instance){
    #ifdef HAVE_TIM6
    if(instance == TIM6) return true;
    #endif
    #ifdef HAVE_TIM7
    if(instance == TIM7) return true;
    #endif
    return false;
}

bool TimerUtils::isGenericTimer(const TIM_TypeDef * instance){
    #ifdef HAVE_TIM2
    if(instance == TIM2) return true;
    #endif
    #ifdef HAVE_TIM3
    if(instance == TIM3) return true;
    #endif
    #ifdef HAVE_TIM4
    if(instance == TIM4) return true;
    #endif
    #ifdef HAVE_TIM5
    if(instance == TIM5) return true;
    #endif
    return false;
}

bool TimerUtils::isAdvancedTimer(const TIM_TypeDef * instance){
    #ifdef HAVE_TIM1
    if(instance == TIM1) return true;
    #endif
    #ifdef HAVE_TIM8
    if(instance == TIM8) return true;
    #endif
    #ifdef HAVE_TIM9
    if(instance == TIM9) return true;
    #endif
    #ifdef HAVE_TIM10
    if(instance == TIM10) return true;
    #endif
    return false;
}

IRQn TimerUtils::ItToIrq(const TIM_TypeDef * instance, const TimerIT & it){
    switch((uint32_t)instance){
        #ifdef HAVE_TIM1
        case TIM1_BASE:
            switch(it){
                case TimerIT::Update:
                    return TIM1_UP_IRQn;
                case TimerIT::CC1:
                case TimerIT::CC2:
                case TimerIT::CC3:
                case TimerIT::CC4:
                    return TIM1_CC_IRQn;
                case TimerIT::Trigger:
                case TimerIT::COM:
                    return TIM1_TRG_COM_IRQn;
                case TimerIT::Break:
                    return TIM1_BRK_IRQn;
            }
            break;
        #endif

        #ifdef HAVE_TIM2
        case TIM2_BASE:
            return TIM2_IRQn;
        #endif

        #ifdef HAVE_TIM3
        case TIM3_BASE:
            return TIM3_IRQn;
        #endif

        #ifdef HAVE_TIM4
        case TIM4_BASE:
            return TIM4_IRQn;
        #endif

        default:
            break;
    }
    return Software_IRQn;
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

Gpio & TimerUtils::getPin(const TIM_TypeDef * instance, const Channel channel){
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

    return(*gpio_port)[(Pin)gpio_pin];
}
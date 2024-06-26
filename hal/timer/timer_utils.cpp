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

IRQn TimerUtils::ItToIrq(const TIM_TypeDef * instance, const IT & it){
    switch((uint32_t)instance){
        #ifdef HAVE_TIM1
        case TIM1_BASE:
            switch(it){
                case IT::Update:
                    return TIM1_UP_IRQn;
                case IT::CC1:
                case IT::CC2:
                case IT::CC3:
                case IT::CC4:
                    return TIM1_CC_IRQn;
                case IT::Trigger:
                case IT::COM:
                    return TIM1_TRG_COM_IRQn;
                case IT::Break:
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
            return TIM##x##_CH1_Gpio;\
        case Channel::CH1N:\
            return TIM##x##_CH1N_Gpio;\
        case Channel::CH2:\
            return TIM##x##_CH2_Gpio;\
        case Channel::CH2N:\
            return TIM##x##_CH2N_Gpio;\
        case Channel::CH3:\
            return TIM##x##_CH3_Gpio;\
        case Channel::CH3N:\
            return TIM##x##_CH3N_Gpio;\
        case Channel::CH4:\
            return TIM##x##_CH4_Gpio;\
    }\
    break;\

#define GENERIC_TIMER_GPIO_TEMPLATE(x)\
case TIM##x##_BASE:\
    switch(channel){\
        default:\
        case Channel::CH1:\
            return TIM##x##_CH1_Gpio;\
        case Channel::CH2:\
            return TIM##x##_CH2_Gpio;\
        case Channel::CH3:\
            return TIM##x##_CH3_Gpio;\
        case Channel::CH4:\
            return TIM##x##_CH4_Gpio;\
    }\
    break;\

Gpio & TimerUtils::getPin(const TIM_TypeDef * instance, const Channel channel){
    switch((uint32_t)instance){
        default:
            return GpioNull;

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
}
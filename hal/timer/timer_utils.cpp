#include "timer_utils.hpp"

using namespace yumud;


bool TimerUtils::isBasicTimer(const TIM_TypeDef * instance){
    switch((uint32_t)instance){
        #ifdef HAVE_TIM6
        case TIM6_BASE: return true;
        #endif

        #ifdef HAVE_TIM7
        case TIM7_BASE: return true;
        #endif

        default:return false;
    }
}

bool TimerUtils::isGenericTimer(const TIM_TypeDef * instance){
    switch((uint32_t)instance){
        #ifdef HAVE_TIM2
        case TIM2_BASE: return true;
        #endif

        #ifdef HAVE_TIM3
        case TIM3_BASE: return true;
        #endif

        #ifdef HAVE_TIM4
        case TIM4_BASE: return true;
        #endif

        #ifdef HAVE_TIM5
        case TIM5_BASE: return true;
        #endif
    
        default:
            return false;
    }
}

bool TimerUtils::isAdvancedTimer(const TIM_TypeDef * instance){
    switch((uint32_t)instance){
        #ifdef HAVE_TIM1
        case TIM1_BASE: return true;
        #endif

        #ifdef HAVE_TIM8
        case TIM8_BASE: return true;
        #endif

        #ifdef HAVE_TIM9
        case TIM9_BASE: return true;
        #endif

        #ifdef HAVE_TIM10
        case TIM10_BASE: return true;
        #endif
    
        default:
            return false;
    }
}

IRQn TimerUtils::ItToIrq(const TIM_TypeDef * instance, const IT & it){

    #define GENERIC_TIMER_IRQ_TEMPLATE(x)\
    case TIM##x##_BASE:\
        return TIM##x##_IRQn;\

    #define ADVANCED_TIMER_IRQ_TEMPLATE(x)\
    case TIM##x##_BASE:\
        switch(it){\
            case IT::Update:\
                return TIM##x##_UP_IRQn;\
            case IT::CC1:\
            case IT::CC2:\
            case IT::CC3:\
            case IT::CC4:\
                return TIM##x##_CC_IRQn;\
            case IT::Trigger:\
            case IT::COM:\
                return TIM##x##_TRG_COM_IRQn;\
            case IT::Break:\
                return TIM##x##_BRK_IRQn;\
        }\
        break;\

    switch((uint32_t)instance){
        #ifdef HAVE_TIM1
        ADVANCED_TIMER_IRQ_TEMPLATE(1)
        #endif

        #ifdef HAVE_TIM2
        GENERIC_TIMER_IRQ_TEMPLATE(2)
        #endif

        #ifdef HAVE_TIM3
        GENERIC_TIMER_IRQ_TEMPLATE(3)
        #endif

        #ifdef HAVE_TIM4
        GENERIC_TIMER_IRQ_TEMPLATE(4)
        #endif

        #ifdef HAVE_TIM5
        GENERIC_TIMER_IRQ_TEMPLATE(5)
        #endif

        #ifdef HAVE_TIM6
        GENERIC_TIMER_IRQ_TEMPLATE(6)
        #endif

        #ifdef HAVE_TIM7
        GENERIC_TIMER_IRQ_TEMPLATE(7)
        #endif

        #ifdef HAVE_TIM8
        ADVANCED_TIMER_IRQ_TEMPLATE(8)
        #endif

        #ifdef HAVE_TIM9
        ADVANCED_TIMER_IRQ_TEMPLATE(9)
        #endif

        #ifdef HAVE_TIM10
        ADVANCED_TIMER_IRQ_TEMPLATE(10)
        #endif

        default:
            break;
    }
    return Software_IRQn;

    #undef GENERIC_TIMER_IRQ_TEMPLATE
    #undef ADVANCED_TIMER_IRQ_TEMPLATE
}


Gpio & TimerUtils::getPin(const TIM_TypeDef * instance, const Channel channel){    
    #define ADVANCED_TIMER_GPIO_TEMPLATE(x)\
    case TIM##x##_BASE:\
        switch(channel){\
            default:\
            case Channel::CH1:\
                return TIM##x##_CH1_GPIO;\
            case Channel::CH1N:\
                return TIM##x##_CH1N_GPIO;\
            case Channel::CH2:\
                return TIM##x##_CH2_GPIO;\
            case Channel::CH2N:\
                return TIM##x##_CH2N_GPIO;\
            case Channel::CH3:\
                return TIM##x##_CH3_GPIO;\
            case Channel::CH3N:\
                return TIM##x##_CH3N_GPIO;\
            case Channel::CH4:\
                return TIM##x##_CH4_GPIO;\
        }\
        break;\

    #define GENERIC_TIMER_GPIO_TEMPLATE(x)\
    case TIM##x##_BASE:\
        switch(channel){\
            default:\
            case Channel::CH1:\
                return TIM##x##_CH1_GPIO;\
            case Channel::CH2:\
                return TIM##x##_CH2_GPIO;\
            case Channel::CH3:\
                return TIM##x##_CH3_GPIO;\
            case Channel::CH4:\
                return TIM##x##_CH4_GPIO;\
        }\
        break;\

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

        //basic timer has no io
    
        #ifdef HAVE_TIM8
        ADVANCED_TIMER_GPIO_TEMPLATE(8)
        #endif

        #ifdef HAVE_TIM9
        ADVANCED_TIMER_GPIO_TEMPLATE(9)
        #endif

        #ifdef HAVE_TIM10
        ADVANCED_TIMER_GPIO_TEMPLATE(10)
        #endif
    }

    #undef ADVANCED_TIMER_GPIO_TEMPLATE
    #undef GENERIC_TIMER_GPIO_TEMPLATE
}
#include "timer_utils.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;

namespace ymd::hal::details{
bool is_basic_timer(const TIM_TypeDef * inst){
    switch(reinterpret_cast<uint32_t>(inst)){
        #ifdef ENABLE_TIM6
        case TIM6_BASE: return true;
        #endif

        #ifdef ENABLE_TIM7
        case TIM7_BASE: return true;
        #endif

        default:return false;
    }
}

bool is_generic_timer(const TIM_TypeDef * inst){
    switch(reinterpret_cast<uint32_t>(inst)){
        #ifdef ENABLE_TIM2
        case TIM2_BASE: return true;
        #endif

        #ifdef ENABLE_TIM3
        case TIM3_BASE: return true;
        #endif

        #ifdef ENABLE_TIM4
        case TIM4_BASE: return true;
        #endif

        #ifdef ENABLE_TIM5
        case TIM5_BASE: return true;
        #endif
    
        default:
            return false;
    }
}

bool is_advanced_timer(const TIM_TypeDef * inst){
    switch(reinterpret_cast<uint32_t>(inst)){
        #ifdef ENABLE_TIM1
        case TIM1_BASE: return true;
        #endif

        #ifdef ENABLE_TIM8
        case TIM8_BASE: return true;
        #endif

        #ifdef ENABLE_TIM9
        case TIM9_BASE: return true;
        #endif

        #ifdef ENABLE_TIM10
        case TIM10_BASE: return true;
        #endif
    
        default:
            return false;
    }
}

IRQn it_to_irq(const TIM_TypeDef * inst, const TimerIT it){
    using enum TimerChannelIndex;

    #define GENERIC_TIMER_IRQ_TEMPLATE(x)\
    case TIM##x##_BASE:\
        return TIM##x##_IRQn;\

    #define ADVANCED_TIMER_IRQ_TEMPLATE(x)\
    case TIM##x##_BASE:\
        switch(it){\
            case TimerIT::Update:\
                return TIM##x##_UP_IRQn;\
            case TimerIT::CC1:\
            case TimerIT::CC2:\
            case TimerIT::CC3:\
            case TimerIT::CC4:\
                return TIM##x##_CC_IRQn;\
            case TimerIT::Trigger:\
            case TimerIT::COM:\
                return TIM##x##_TRG_COM_IRQn;\
            case TimerIT::Break:\
                return TIM##x##_BRK_IRQn;\
        }\
        break;\

    switch(reinterpret_cast<uint32_t>(inst)){
        #ifdef ENABLE_TIM1
        ADVANCED_TIMER_IRQ_TEMPLATE(1)
        #endif

        #ifdef ENABLE_TIM2
        GENERIC_TIMER_IRQ_TEMPLATE(2)
        #endif

        #ifdef ENABLE_TIM3
        GENERIC_TIMER_IRQ_TEMPLATE(3)
        #endif

        #ifdef ENABLE_TIM4
        GENERIC_TIMER_IRQ_TEMPLATE(4)
        #endif

        #ifdef ENABLE_TIM5
        GENERIC_TIMER_IRQ_TEMPLATE(5)
        #endif

        #ifdef ENABLE_TIM6
        GENERIC_TIMER_IRQ_TEMPLATE(6)
        #endif

        #ifdef ENABLE_TIM7
        GENERIC_TIMER_IRQ_TEMPLATE(7)
        #endif

        #ifdef ENABLE_TIM8
        ADVANCED_TIMER_IRQ_TEMPLATE(8)
        #endif

        #ifdef ENABLE_TIM9
        ADVANCED_TIMER_IRQ_TEMPLATE(9)
        #endif

        #ifdef ENABLE_TIM10
        ADVANCED_TIMER_IRQ_TEMPLATE(10)
        #endif

        default:
            break;
    }
    return Software_IRQn;

    #undef GENERIC_TIMER_IRQ_TEMPLATE
    #undef ADVANCED_TIMER_IRQ_TEMPLATE
}


Gpio & get_pin(const TIM_TypeDef * inst, const TimerChannelIndex channel){    
    using enum TimerChannelIndex;

    #define ADVANCED_TIMER_GPIO_TEMPLATE(x)\
    case TIM##x##_BASE:\
        switch(channel){\
            default:\
            case CH1:\
                return TIM##x##_CH1_GPIO;\
            case CH1N:\
                return TIM##x##_CH1N_GPIO;\
            case CH2:\
                return TIM##x##_CH2_GPIO;\
            case CH2N:\
                return TIM##x##_CH2N_GPIO;\
            case CH3:\
                return TIM##x##_CH3_GPIO;\
            case CH3N:\
                return TIM##x##_CH3N_GPIO;\
            case CH4:\
                return TIM##x##_CH4_GPIO;\
        }\
        break;\

    #define GENERIC_TIMER_GPIO_TEMPLATE(x)\
    case TIM##x##_BASE:\
        switch(channel){\
            default:\
            case CH1:\
                return TIM##x##_CH1_GPIO;\
            case CH2:\
                return TIM##x##_CH2_GPIO;\
            case CH3:\
                return TIM##x##_CH3_GPIO;\
            case CH4:\
                return TIM##x##_CH4_GPIO;\
        }\
        break;\

    switch(reinterpret_cast<uint32_t>(inst)){
        default:
            return NullGpio;

        #ifdef ENABLE_TIM1
        ADVANCED_TIMER_GPIO_TEMPLATE(1)
        #endif

        #ifdef ENABLE_TIM2
        GENERIC_TIMER_GPIO_TEMPLATE(2)
        #endif

        #ifdef ENABLE_TIM3
        GENERIC_TIMER_GPIO_TEMPLATE(3)
        #endif

        #ifdef ENABLE_TIM4
        GENERIC_TIMER_GPIO_TEMPLATE(4)
        #endif

        #ifdef ENABLE_TIM5
        GENERIC_TIMER_GPIO_TEMPLATE(5)
        #endif

        //basic timer has no io
    
        #ifdef ENABLE_TIM8
        ADVANCED_TIMER_GPIO_TEMPLATE(8)
        #endif

        #ifdef ENABLE_TIM9
        ADVANCED_TIMER_GPIO_TEMPLATE(9)
        #endif

        #ifdef ENABLE_TIM10
        ADVANCED_TIMER_GPIO_TEMPLATE(10)
        #endif
    }

    #undef ADVANCED_TIMER_GPIO_TEMPLATE
    #undef GENERIC_TIMER_GPIO_TEMPLATE
}

}
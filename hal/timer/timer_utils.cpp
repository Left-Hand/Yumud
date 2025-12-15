#include "timer_utils.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "timer_layout.hpp"
#include "core/sdk.hpp"


using namespace ymd;
using namespace ymd::hal;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, TIM_TypeDef *)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))



namespace ymd::hal::timer::details{
bool is_basic_timer(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef TIM6_PRESENT
        case TIM6_BASE: return true;
        #endif

        #ifdef TIM7_PRESENT
        case TIM7_BASE: return true;
        #endif

        default:return false;
    }
}

bool is_generic_timer(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef TIM2_PRESENT
        case TIM2_BASE: return true;
        #endif

        #ifdef TIM3_PRESENT
        case TIM3_BASE: return true;
        #endif

        #ifdef TIM4_PRESENT
        case TIM4_BASE: return true;
        #endif

        #ifdef TIM5_PRESENT
        case TIM5_BASE: return true;
        #endif
    }
    return false;
}

bool is_advanced_timer(const void * inst){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef TIM1_PRESENT
        case TIM1_BASE: return true;
        #endif

        #ifdef TIM8_PRESENT
        case TIM8_BASE: return true;
        #endif

        #ifdef TIM9_PRESENT
        case TIM9_BASE: return true;
        #endif

        #ifdef TIM10_PRESENT
        case TIM10_BASE: return true;
        #endif
    
        default:
            return false;
    }
}

IRQn it_to_irq(const void * inst, const TimerIT it){

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

    switch(reinterpret_cast<size_t>(inst)){
        #ifdef TIM1_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(1)
        #endif

        #ifdef TIM2_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(2)
        #endif

        #ifdef TIM3_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(3)
        #endif

        #ifdef TIM4_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(4)
        #endif

        #ifdef TIM5_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(5)
        #endif

        #ifdef TIM6_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(6)
        #endif

        #ifdef TIM7_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(7)
        #endif

        #ifdef TIM8_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(8)
        #endif

        #ifdef TIM9_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(9)
        #endif

        #ifdef TIM10_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(10)
        #endif

        default:
            break;
    }
    return Software_IRQn;

    #undef GENERIC_TIMER_IRQ_TEMPLATE
    #undef ADVANCED_TIMER_IRQ_TEMPLATE
}


std::tuple<uint16_t, uint16_t> calc_arr_and_psc(
    const uint32_t aligned_bus_clk_freq,
    const TimerCountFreq count_freq
){
    if(count_freq.is<NearestFreq>()){
        const auto arr_and_psc = ArrAndPsc::from_nearest_count_freq(
            aligned_bus_clk_freq,
            count_freq.unwrap_as<NearestFreq>().count, 
            {0, 65535}
        );
        // PANIC(arr_and_psc.arr, arr_and_psc.psc);
        return std::make_tuple(arr_and_psc.arr, arr_and_psc.psc);
    }else if(count_freq.is<ArrAndPsc>()){
        const auto arr_and_psc = count_freq.unwrap_as<ArrAndPsc>();
        return std::make_tuple(arr_and_psc.arr, arr_and_psc.psc);
    }else{
        __builtin_trap();
    }
}

void set_remap(void* inst_, const TimerRemap rm){
    switch(reinterpret_cast<size_t>(inst_)){
    #ifdef TIM1_PRESENT
    case TIM1_BASE:
        switch(rm){
            case TimerRemap::_0:
                GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, DISABLE);
                return;
            case TimerRemap::_1:
                GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, ENABLE);
                return;
            case TimerRemap::_2:
                GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, DISABLE);
                return;
            case TimerRemap::_3:
                GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);
                return;
        }
        break;
    #endif

    #ifdef TIM2_PRESENT
    case TIM2_BASE:
        switch(rm){
            case TimerRemap::_0:
                return;
            case TimerRemap::_1:
                GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
                return;
            case TimerRemap::_2:
                GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
                return;
            case TimerRemap::_3:
                GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
                return;
        }
        break;
    #endif

    #ifdef TIM3_PRESENT
    case TIM3_BASE:
        switch(rm){
            case TimerRemap::_0:
                return;
            case TimerRemap::_1:
                return;
            case TimerRemap::_2:
                GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
                return;
            case TimerRemap::_3:
                GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
                return;
        }
        break;
    #endif

    #ifdef TIM4_PRESENT
    case TIM4_BASE:
        switch(rm){
            case TimerRemap::_0:
                GPIO_PinRemapConfig(GPIO_Remap_TIM4, DISABLE);
                return;
            case TimerRemap::_1:
                GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);
                return;
            default: break;
        }
        break;
    #endif

    #ifdef TIM5_PRESENT
    case TIM5_BASE:
        //no remap
        switch(rm){
            case TimerRemap::_0:
                return;
            default: break;
        }
        break;
    #endif
    
    #ifdef TIM6_PRESENT
    case TIM6_BASE:
        //no remap
        switch(rm){
            case TimerRemap::_0:
                return;
            default: break;
        }
        break;
    #endif
    #ifdef TIM7_PRESENT
    case TIM7_BASE:
        //no remap
        switch(rm){
            case TimerRemap::_0:
                return;
            default: break;
        }
        break;
    #endif

    #ifdef TIM8_PRESENT
    case TIM8_BASE:
        switch(rm){
            case TimerRemap::_0:
                return;
            case TimerRemap::_1:
                GPIO_PinRemapConfig(GPIO_Remap_TIM8, ENABLE);
                return;
            default:
                break;
        }
        break;
    #endif
    }
    __builtin_trap();
}

void enable_rcc(void* inst_, const Enable en){
    switch(reinterpret_cast<size_t>(inst_)){
    #ifdef TIM1_PRESENT
    case TIM1_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, en == EN);
        return;
    #endif

    #ifdef TIM2_PRESENT
    case TIM2_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, en == EN);
        return;
    #endif

    #ifdef TIM3_PRESENT
    case TIM3_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, en == EN);
        return;
    #endif

    #ifdef TIM4_PRESENT
    case TIM4_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, en == EN);
        return;
    #endif

    #ifdef TIM5_PRESENT
    case TIM5_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, en == EN);
        return;
    #endif

    
    #ifdef TIM6_PRESENT
    case TIM6_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, en == EN);
        return;
    #endif

    #ifdef TIM7_PRESENT
    case TIM7_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, en == EN);
        return;
    #endif

    #ifdef TIM8_PRESENT
    case TIM8_BASE:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, en == EN);
        return;
    #endif
    }
    __builtin_trap();
}

}
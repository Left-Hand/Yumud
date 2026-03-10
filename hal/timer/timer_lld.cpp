#include "timer_lld.hpp"
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



namespace ymd::lld{

Nth timer_to_nth(const uintptr_t inst_base){
    switch(inst_base){
        #ifdef TIM1_PRESENT
        case TIM1_BASE: return Nth{1}; 
        #endif

        #ifdef TIM2_PRESENT
        case TIM2_BASE: return Nth{2}; 
        #endif

        #ifdef TIM3_PRESENT
        case TIM3_BASE: return Nth{3}; 
        #endif

        #ifdef TIM4_PRESENT
        case TIM4_BASE: return Nth{4}; 
        #endif

        #ifdef TIM5_PRESENT
        case TIM5_BASE: return Nth{5}; 
        #endif

        #ifdef TIM6_PRESENT
        case TIM6_BASE: return Nth{6}; 
        #endif

        #ifdef TIM7_PRESENT
        case TIM7_BASE: return Nth{7}; 
        #endif

        #ifdef TIM8_PRESENT
        case TIM8_BASE: return Nth{8}; 
        #endif

        #ifdef TIM9_PRESENT
        case TIM9_BASE: return Nth{9}; 
        #endif

        #ifdef TIM10_PRESENT
        case TIM10_BASE: return Nth{10}; 
        #endif
    }

    __builtin_trap();
}


std::tuple<uint16_t, uint16_t> timer_calc_arr_and_psc(
    const uint32_t aligned_bus_clk_freq,
    const TimerCountFreq count_freq
){
    if(count_freq.is<NearestFreq>()){
        const auto arr_and_psc = hal::timer::ArrAndPsc::from_nearest_count_freq(
            aligned_bus_clk_freq,
            count_freq.unwrap_as<NearestFreq>().count, 
            {0, 65535}
        );
        // PANIC(arr_and_psc.arr, arr_and_psc.psc);
        return std::make_tuple(arr_and_psc.arr, arr_and_psc.psc);
    }else if(count_freq.is<hal::timer::ArrAndPsc>()){
        const auto arr_and_psc = count_freq.unwrap_as<hal::timer::ArrAndPsc>();
        return std::make_tuple(arr_and_psc.arr, arr_and_psc.psc);
    }else{
        __builtin_trap();
    }
}

void timer_set_remap(const Nth nth, const TimerRemap rm){
    switch(nth.count()){
    #ifdef TIM1_PRESENT
    case 1:
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
    case 2:
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
    case 3:
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
    case 4:
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
    case 5:
        //no remap
        switch(rm){
            case TimerRemap::_0:
                return;
            default: break;
        }
        break;
    #endif
    
    #ifdef TIM6_PRESENT
    case 6:
        //no remap
        switch(rm){
            case TimerRemap::_0:
                return;
            default: break;
        }
        break;
    #endif
    #ifdef TIM7_PRESENT
    case 7:
        //no remap
        switch(rm){
            case TimerRemap::_0:
                return;
            default: break;
        }
        break;
    #endif

    #ifdef TIM8_PRESENT
    case 8:
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

void timer_enable_rcc(const Nth nth, const Enable en){
    switch(nth.count()){
    #ifdef TIM1_PRESENT
    case 1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, (en == EN));
        return;
    #endif

    #ifdef TIM2_PRESENT
    case 2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, (en == EN));
        return;
    #endif

    #ifdef TIM3_PRESENT
    case 3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, (en == EN));
        return;
    #endif

    #ifdef TIM4_PRESENT
    case 4:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, (en == EN));
        return;
    #endif

    #ifdef TIM5_PRESENT
    case 5:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, (en == EN));
        return;
    #endif

    
    #ifdef TIM6_PRESENT
    case 6:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, (en == EN));
        return;
    #endif

    #ifdef TIM7_PRESENT
    case 7:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, (en == EN));
        return;
    #endif

    #ifdef TIM8_PRESENT
    case 8:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, (en == EN));
        return;
    #endif
    }
    __builtin_trap();
}

}
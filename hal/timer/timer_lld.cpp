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

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, TIM_TypeDef)>(x))
// #define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::Tim)>(x))

namespace{

template<typename T>
static void reg_set_or_clear_bit(volatile T & reg, const T mask, const bool en){
    if(en){
        reg = static_cast<T>(reg) | static_cast<T>(mask);
    }else{
        reg = static_cast<T>(reg) & static_cast<T>(~mask);
    }
}

template<typename T>
[[nodiscard]] static bool reg_get_bit(const T  reg, const T mask){
    return static_cast<T>(reg) & mask;
}


}

namespace ymd::lld{


    
void timeroc_set_oc_mode(
    void * p_inst,
    const TimerChannelSelection ch_sel, 
    const TimerOcMode mode
){
    using ChannelSelection = TimerChannelSelection;
    const uint8_t bits = std::bit_cast<uint8_t>(mode) << 4;

    switch(ch_sel.kind()){
        case ChannelSelection::CH1:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR1;
            const uint16_t m_code = TIM_OC1M;
            const uint16_t s_code = TIM_CC1S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(bits);
            SPL_INST(p_inst)->CHCTLR1 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH2:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR1;
            const uint16_t m_code = TIM_OC2M;
            const uint16_t s_code = TIM_CC2S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(bits) << 8);
            SPL_INST(p_inst)->CHCTLR1 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH3:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR2;
            const uint16_t m_code = TIM_OC3M;
            const uint16_t s_code = TIM_CC3S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(bits);
            SPL_INST(p_inst)->CHCTLR2 = tmpccmrx;
            break;
        }
        case ChannelSelection::CH4:{
            uint16_t tmpccmrx = SPL_INST(p_inst)->CHCTLR2;
            const uint16_t m_code = TIM_OC4M;
            const uint16_t s_code = TIM_CC4S;
            tmpccmrx &= uint16_t(~(uint16_t(m_code << 8)));
            tmpccmrx &= uint16_t(~(uint16_t(s_code)));
            tmpccmrx |= uint16_t(uint16_t(bits) << 8);
            SPL_INST(p_inst)->CHCTLR2 = tmpccmrx;
            break;
        }
        default:
            __builtin_trap();
            break;
    }
}



void timeroc_enable_output(
    void * p_inst,
    const TimerChannelSelection ch_sel,
    const Enable en
){
    if(en == EN) SPL_INST(p_inst)->CCER |= (1 << (std::bit_cast<uint8_t>(ch_sel) * 2));
    else SPL_INST(p_inst)->CCER &= (~(1 << ((std::bit_cast<uint8_t>(ch_sel)) * 2)));
}



void timeroc_enable_cvr_sync(
    void * p_inst,
    const TimerChannelSelection ch_sel,
    const Enable en
){
    using ChannelSelection = TimerChannelSelection;
    const auto e_code = (en == EN) ? TIM_OCPreload_Enable : TIM_OCPreload_Disable;
    switch(ch_sel.kind()){
        case ChannelSelection::CH1:
            TIM_OC1PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        case ChannelSelection::CH2:
            TIM_OC2PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        case ChannelSelection::CH3:
            TIM_OC3PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        case ChannelSelection::CH4:
            TIM_OC4PreloadConfig(SPL_INST(p_inst), e_code);
            break;
        default:
            __builtin_trap();
            break;
    }
}

void timeroc_set_valid_level(
    void * p_inst,
    const TimerChannelSelection ch_sel,
    const BoolLevel level
){
    if(level == LOW) SPL_INST(p_inst)->CCER |= (1 << (std::bit_cast<uint8_t>(ch_sel) * 2 + 1));
    else SPL_INST(p_inst)->CCER &= (~(1 << ((std::bit_cast<uint8_t>(ch_sel)) * 2 + 1)));
}

bool timer_is_up_counting(void * p_inst){
    auto p_reg = (volatile uint16_t *)(&SPL_INST(p_inst)->CTLR1);
    return reg_get_bit(uint16_t(*p_reg), uint16_t(TIM_DIR)) == 0;
}


Nth timer_to_nth(const uintptr_t p_inst_base){
    switch(p_inst_base){
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
        const auto arr_and_psc = hal::timer::ArrAndPsc::from_nearest_freq(
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

volatile uint16_t & timer_channel_find_cvr(
    void * p_inst, 
    const hal::TimerChannelSelection sel
){ 
    switch(sel.kind()){
        default: __builtin_trap();
        case hal::TimerChannelSelection::CH1:
        case hal::TimerChannelSelection::CH1N:
            return (SPL_INST(p_inst)->CH1CVR);
        case hal::TimerChannelSelection::CH2:
        case hal::TimerChannelSelection::CH2N:
            return (SPL_INST(p_inst)->CH2CVR);
        case hal::TimerChannelSelection::CH3:
        case hal::TimerChannelSelection::CH3N:
            return (SPL_INST(p_inst)->CH3CVR);
        case hal::TimerChannelSelection::CH4:
            return (SPL_INST(p_inst)->CH4CVR);
    }
    __builtin_trap();
}



void timer_channel_enable_dma(void * p_inst, const hal::TimerChannelSelection sel, const Enable en){
    const uint16_t source = [&] -> uint16_t{
        switch(sel.kind()){
        case hal::TimerChannelSelection::CH1:
            return TIM_DMA_CC1;
        case hal::TimerChannelSelection::CH2:
            return TIM_DMA_CC2;
        case hal::TimerChannelSelection::CH3:
            return TIM_DMA_CC3;
        case hal::TimerChannelSelection::CH4:
            return TIM_DMA_CC4;
        default:
            __builtin_trap();
        }
    }();

    TIM_DMACmd(SPL_INST(p_inst), source, (en == EN));
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


    #ifdef GPIO_PartialRemap_TIM9
    case 9:
        switch(rm){
            case TimerRemap::_0:
                return;
            case TimerRemap::_1:
                return;
            case TimerRemap::_2:
                GPIO_PinRemapConfig(GPIO_PartialRemap_TIM9, ENABLE);
                return;
            case TimerRemap::_3:
                GPIO_PinRemapConfig(GPIO_FullRemap_TIM9, ENABLE);
                return;
        }
        break;
    #endif

    #ifdef GPIO_PartialRemap_TIM10
    case 10:
        switch(rm){
            case TimerRemap::_0:
                return;
            case TimerRemap::_1:
                return;
            case TimerRemap::_2:
                GPIO_PinRemapConfig(GPIO_PartialRemap_TIM10, ENABLE);
                return;
            case TimerRemap::_3:
                GPIO_PinRemapConfig(GPIO_FullRemap_TIM10, ENABLE);
                return;
        }
        break;
    #endif
    }
    __builtin_trap();
}

void timer_enable_rcc(const Nth nth, const Enable en){
    switch(nth.count()){
    #ifdef RCC_APB2Periph_TIM1
    case 1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, (en == EN));
        return;
    #endif

    #ifdef RCC_APB1Periph_TIM2
    case 2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, (en == EN));
        return;
    #endif

    #ifdef RCC_APB1Periph_TIM3
    case 3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, (en == EN));
        return;
    #endif

    #ifdef RCC_APB1Periph_TIM4
    case 4:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, (en == EN));
        return;
    #endif

    #ifdef RCC_APB1Periph_TIM4
    case 5:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, (en == EN));
        return;
    #endif

    
    #ifdef RCC_APB1Periph_TIM6
    case 6:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, (en == EN));
        return;
    #endif

    #ifdef RCC_APB1Periph_TIM7
    case 7:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, (en == EN));
        return;
    #endif

    #ifdef RCC_APB2Periph_TIM9
    case 8:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, (en == EN));
        return;
    #endif

    #ifdef RCC_APB2Periph_TIM9
    case 9:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, (en == EN));
        return;
    #endif

    #ifdef RCC_APB2Periph_TIM10
    case 10:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, (en == EN));
        return;
    #endif

    }
    __builtin_trap();
}

}
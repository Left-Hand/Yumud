#include "timer_channel.hpp"
#include "hal/dma/dma.hpp"
#include "timer_layout.hpp"
#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, TIM_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


volatile uint16_t & TimerChannel::from_channel_to_cvr(
    void * inst, 
    const ChannelSelection sel
){

    switch(sel.kind()){
        default: __builtin_trap();
        case ChannelSelection::CH1:
        case ChannelSelection::CH1N:
            return (SPL_INST(inst)->CH1CVR);
        case ChannelSelection::CH2:
        case ChannelSelection::CH2N:
            return (SPL_INST(inst)->CH2CVR);
        case ChannelSelection::CH3:
        case ChannelSelection::CH3N:
            return (SPL_INST(inst)->CH3CVR);
        case ChannelSelection::CH4:
            return (SPL_INST(inst)->CH4CVR);
    }
}


void TimerChannel::enable_dma(const Enable en){

    const uint16_t source = [&] -> uint16_t{
        switch(sel_.kind()){
        case ChannelSelection::CH1:
            return TIM_DMA_CC1;
        case ChannelSelection::CH2:
            return TIM_DMA_CC2;
        case ChannelSelection::CH3:
            return TIM_DMA_CC3;
        case ChannelSelection::CH4:
            return TIM_DMA_CC4;
        default:
            __builtin_trap();
        }
    }();

    TIM_DMACmd(SPL_INST(inst_), source, (en == EN));
}


static Option<DmaChannel &> timch_to_dma(const Nth nth, timer::ChannelSelection::Kind ch_sel) {
    using Kind = timer::ChannelSelection::Kind;
    #define FULL_DMA_CASE(x)\
        case x:\
        switch(ch_sel){\
            case Kind::CH1:\
                return Some(&TIM##x##_CH1_DMA_CH);\
            case Kind::CH2:\
                return Some(&TIM##x##_CH2_DMA_CH);\
            case Kind::CH3:\
                return Some(&TIM##x##_CH3_DMA_CH);\
            case Kind::CH4:\
                return Some(&TIM##x##_CH4_DMA_CH);\
            default:\
                break;\
        }\
        break;\
        
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        FULL_DMA_CASE(1)
        #endif

        #ifdef TIM2_PRESENT
        FULL_DMA_CASE(2)
        #endif

        #ifdef TIM3_PRESENT
        case TIM3_BASE:
        switch(ch_sel){
            case Kind::CH1:
                return Some(&TIM3_CH1_DMA_CH);
            case Kind::CH3:
                return Some(&TIM3_CH3_DMA_CH);
            case Kind::CH4:
                return Some(&TIM3_CH4_DMA_CH);
            default:
                break;
        }
        break;
        #endif

        #ifdef TIM4_PRESENT
        case TIM4_BASE:
        switch(ch_sel){
            case Kind::CH1:
                return Some(&TIM4_CH1_DMA_CH);
            case Kind::CH2:
                return Some(&TIM4_CH2_DMA_CH);
            case Kind::CH3:
                return Some(&TIM4_CH3_DMA_CH);
            default:
                break;
        }
        break;
        #endif

        #ifdef TIM5_PRESENT
        FULL_DMA_CASE(5)
        #endif
        #ifdef TIM8_PRESENT
        FULL_DMA_CASE(8)
        #endif
        #ifdef TIM9_PRESENT
        FULL_DMA_CASE(9)
        #endif
        #ifdef TIM10_PRESENT
        FULL_DMA_CASE(10)
        #endif
    }

    return None;
    #undef FULL_DMA_CASE
}



Option<DmaChannel &> TimerChannel::dma() const {
    return timch_to_dma(lld::timer_to_nth(reinterpret_cast<uintptr_t>(inst_)), sel_.kind());
}




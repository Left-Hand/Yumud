#include "timer_channel.hpp"
#include "hal/dma/dma.hpp"

using namespace ymd;
using namespace ymd::hal;

volatile uint16_t & TimerChannel::from_channel_to_cvr(
    TIM_TypeDef * timer, 
    const ChannelSelection ch_sel
){

    switch(ch_sel.kind()){
        default: __builtin_trap();
        case ChannelSelection::CH1:
        case ChannelSelection::CH1N:
            return (timer->CH1CVR);
        case ChannelSelection::CH2:
        case ChannelSelection::CH2N:
            return (timer->CH2CVR);
        case ChannelSelection::CH3:
        case ChannelSelection::CH3N:
            return (timer->CH3CVR);
        case ChannelSelection::CH4:
            return (timer->CH4CVR);
    }
}


TimerChannel & TimerChannel::enable_dma(const Enable en){

    const uint16_t source = [&] -> uint16_t{
        switch(ch_sel_.kind()){
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

    TIM_DMACmd(inst_, source, en == EN);

    return *this;
}


DmaChannel & TimerChannel::dma() const {

    #define DMA_NULL dma1_ch1

    #define FULL_DMA_CASE(x)\
        case TIM##x##_BASE:\
        switch(ch_sel_.kind()){\
            case ChannelSelection::CH1:\
                return TIM##x##_CH1_DMA_CH;\
            case ChannelSelection::CH2:\
                return TIM##x##_CH2_DMA_CH;\
            case ChannelSelection::CH3:\
                return TIM##x##_CH3_DMA_CH;\
            case ChannelSelection::CH4:\
                return TIM##x##_CH4_DMA_CH;\
            default:\
                break;\
        }\
        break;\
        
    switch(reinterpret_cast<uint32_t>(inst_)){
        #ifdef ENABLE_TIM1
        FULL_DMA_CASE(1)
        #endif

        #ifdef ENABLE_TIM2
        FULL_DMA_CASE(2)
        #endif

        #ifdef ENABLE_TIM3
        case TIM3_BASE:
        switch(ch_sel_.kind()){
            case ChannelSelection::CH1:
                return TIM3_CH1_DMA_CH;
            case ChannelSelection::CH3:
                return TIM3_CH3_DMA_CH;
            case ChannelSelection::CH4:
                return TIM3_CH4_DMA_CH;
            default:
                break;
        }
        break;
        #endif

        #ifdef ENABLE_TIM4
        case TIM4_BASE:
        switch(ch_sel_.kind()){
            case ChannelSelection::CH1:
                return TIM4_CH1_DMA_CH;
            case ChannelSelection::CH2:
                return TIM4_CH2_DMA_CH;
            case ChannelSelection::CH3:
                return TIM4_CH3_DMA_CH;
            default:
                break;
        }
        break;
        #endif

        #ifdef ENABLE_TIM5
        FULL_DMA_CASE(5)
        #endif
        #ifdef ENABLE_TIM8
        FULL_DMA_CASE(8)
        #endif
        #ifdef ENABLE_TIM9
        FULL_DMA_CASE(9)
        #endif
        #ifdef ENABLE_TIM10
        FULL_DMA_CASE(10)
        #endif
    }

    return DMA_NULL;
    #undef DMA_NULL
    #undef FULL_DMA_CASE
}




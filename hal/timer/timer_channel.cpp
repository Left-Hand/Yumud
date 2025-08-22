#include "timer_channel.hpp"
#include "hal/dma/dma.hpp"

using namespace ymd;
using namespace ymd::hal;

volatile uint16_t & TimerChannel::from_channel_to_cvr(TIM_TypeDef * timer, const ChannelNth _channel){
    using enum ChannelNth;

    switch(_channel){
        default:
        case CH1:
        case CH1N:
            return (timer->CH1CVR);
        case CH2:
        case CH2N:
            return (timer->CH2CVR);
        case CH3:
        case CH3N:
            return (timer->CH3CVR);
        case CH4:
            return (timer->CH4CVR);
    }
}


TimerChannel & TimerChannel::enable_dma(const Enable en){
    using enum ChannelNth;

    uint16_t source = 0;

    switch(idx_){
        case CH1:
            source = TIM_DMA_CC1;
            break;
        case CH2:
            source = TIM_DMA_CC2;
            break;
        case CH3:
            source = TIM_DMA_CC3;
            break;
        case CH4:
            source = TIM_DMA_CC4;
            break;
        default:
            break;
    }

    TIM_DMACmd(inst_, source, en == EN);

    return *this;
}


DmaChannel & TimerChannel::dma() const {
    using enum ChannelNth;

    #define DMA_NULL dma1Ch1

    #define FULL_DMA_CASE(x)\
        case TIM##x##_BASE:\
        switch(idx_){\
            case CH1:\
                return TIM##x##_CH1_DMA_CH;\
            case CH2:\
                return TIM##x##_CH2_DMA_CH;\
            case CH3:\
                return TIM##x##_CH3_DMA_CH;\
            case CH4:\
                return TIM##x##_CH4_DMA_CH;\
            default:\
                break;\
        }\
        break;\
        
    switch((uint32_t)inst_){
        #ifdef ENABLE_TIM1
        FULL_DMA_CASE(1)
        #endif

        #ifdef ENABLE_TIM2
        FULL_DMA_CASE(2)
        #endif

        #ifdef ENABLE_TIM3
        case TIM3_BASE:
        switch(idx_){
            case CH1:
                return TIM3_CH1_DMA_CH;
            case CH3:
                return TIM3_CH3_DMA_CH;
            case CH4:
                return TIM3_CH4_DMA_CH;
            default:
                break;
        }
        break;
        #endif

        #ifdef ENABLE_TIM4
        case TIM4_BASE:
        switch(idx_){
            case CH1:
                return TIM4_CH1_DMA_CH;
            case CH2:
                return TIM4_CH2_DMA_CH;
            case CH3:
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




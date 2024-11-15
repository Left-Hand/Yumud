#include "timer_channel.hpp"
#include "hal/dma/dma.hpp"

using namespace ymd;
volatile uint16_t & TimerChannel::from_channel_to_cvr(const Channel _channel){
    switch(_channel){
        default:
        case Channel::CH1:
        case Channel::CH1N:
            return (instance->CH1CVR);
        case Channel::CH2:
        case Channel::CH2N:
            return (instance->CH2CVR);
        case Channel::CH3:
        case Channel::CH3N:
            return (instance->CH3CVR);
        case Channel::CH4:
            return (instance->CH4CVR);
    }
}


void TimerChannel::enableDma(const bool en){
    uint16_t source = 0;
    switch(channel){
        case Channel::CH1:
            source = TIM_DMA_CC1;
            break;
        case Channel::CH2:
            source = TIM_DMA_CC2;
            break;
        case Channel::CH3:
            source = TIM_DMA_CC3;
            break;
        case Channel::CH4:
            source = TIM_DMA_CC4;
            break;
        default:
            break;
    }
    TIM_DMACmd(instance, source, en);
}


DmaChannel & TimerChannel::dma() const {
    #define DMA_NULL dma1Ch1

    #define FULL_DMA_CASE(x)\
        case TIM##x##_BASE:\
        switch(channel){\
            case Channel::CH1:\
                return TIM##x##_CH1_DMA_CH;\
            case Channel::CH2:\
                return TIM##x##_CH2_DMA_CH;\
            case Channel::CH3:\
                return TIM##x##_CH3_DMA_CH;\
            case Channel::CH4:\
                return TIM##x##_CH4_DMA_CH;\
            default:\
                break;\
        }\
        
    switch((uint32_t)instance){
        #ifdef ENABLE_TIM1
        FULL_DMA_CASE(1)
        #endif

        #ifdef ENABLE_TIM2
        FULL_DMA_CASE(2)
        #endif

        #ifdef ENABLE_TIM3
        case TIM3_BASE:
        switch(channel){
            case Channel::CH1:
                return TIM3_CH1_DMA_CH;
            case Channel::CH3:
                return TIM3_CH3_DMA_CH;
            case Channel::CH4:
                return TIM3_CH4_DMA_CH;
            default:
                break;
        }
        #endif

        #ifdef ENABLE_TIM4
        case TIM4_BASE:
        switch(channel){
            case Channel::CH1:
                return TIM4_CH1_DMA_CH;
            case Channel::CH2:
                return TIM4_CH2_DMA_CH;
            case Channel::CH3:
                return TIM4_CH3_DMA_CH;
            default:
                break;
        }
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




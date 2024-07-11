#include "timer_channel.hpp"

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




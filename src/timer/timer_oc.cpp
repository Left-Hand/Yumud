#include "timer_oc.hpp"

volatile uint16_t * TimerOC::from_channel_to_cvr(const Channel _channel){
    switch(channel){
        default:
        case Channel::CH1:
        case Channel::CH1N:
            return &(base->CH1CVR);
        case Channel::CH2:
        case Channel::CH2N:
            return &(base->CH1CVR);
        case Channel::CH3:
        case Channel::CH3N:
            return &(base->CH1CVR);
        case Channel::CH4:
            return &(base->CH4CVR);
    }
}

TimerOC::TimerOC(TIM_TypeDef * _base, const Channel _channel):base(_base), cvr(*from_channel_to_cvr(_channel)), channel(_channel){
    ;
}

void TimerOC::init(){
    TIM_OCInitTypeDef TIM_OCInitStructure;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    // switch()
    TIM_OC1Init(base,&TIM_OCInitStructure);
}

void TimerOC::enable(const bool en){
    if(en) base->CCER |= 1 << (uint8_t)channel;
    else base->CCER &= ~(1 << (uint8_t)channel);
}
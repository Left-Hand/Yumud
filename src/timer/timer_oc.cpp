#include "timer_oc.hpp"

volatile uint16_t & TimerOC::from_channel_to_cvr(const Channel _channel){
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


TimerOC::TimerOC(TIM_TypeDef * _base, const Channel _channel):instance(_base), cvr(from_channel_to_cvr(_channel)), channel(_channel){
    ;
}

void TimerOC::init(const bool install, const Mode mode){
    TIM_OCInitTypeDef TIM_OCInitStructure;

    TIM_OCInitStructure.TIM_OCMode = (uint16_t)mode;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;


    switch(channel){
        default:
            break;
        case Channel::CH1:
            TIM_OC1Init(instance,&TIM_OCInitStructure);
        case Channel::CH2:
            TIM_OC2Init(instance,&TIM_OCInitStructure);
        case Channel::CH3:
            TIM_OC3Init(instance,&TIM_OCInitStructure);
        case Channel::CH4:
            TIM_OC4Init(instance,&TIM_OCInitStructure);
    }

    setPolarity(true);
    setSync(false);
    if(install){
        installToPin();
    }
}

void TimerOC::enable(const bool en){
    if(en) instance->CCER |= 1 << ((uint8_t)channel * 2);
    else instance->CCER &= ~(1 << ((uint8_t)channel) * 2);
}


void TimerOC::setPolarity(const bool pol){
    if(!pol) instance->CCER |= 1 << ((uint8_t)channel * 2 + 1);
    else instance->CCER &= ~(1 << (((uint8_t)channel) * 2 + 1));
}

void TimerOC::setSync(const bool _sync){
    TIM_ARRPreloadConfig(instance, (FunctionalState)_sync);
}

void TimerOC::setIdleState(const bool state){

}

void TimerOC::installToPin(const bool en){
    GPIO_TypeDef * gpio_port;
    uint16_t gpio_pin = 0;
    switch((uint32_t)instance){
    default:
    case TIM1_BASE:
        switch(channel){
            default:
            case Channel::CH1:
                gpio_port = TIM1_CH1_Port;
                gpio_pin = TIM1_CH1_Pin;
                break;
            case Channel::CH1N:
                gpio_port = TIM1_CH1N_Port;
                gpio_pin = TIM1_CH1N_Pin;
                break;
            case Channel::CH2:
                gpio_port = TIM1_CH2_Port;
                gpio_pin = TIM1_CH2_Pin;
                break;
            case Channel::CH2N:
                gpio_port = TIM1_CH2N_Port;
                gpio_pin = TIM1_CH2N_Pin;
                break;
            case Channel::CH3:
                gpio_port = TIM1_CH3_Port;
                gpio_pin = TIM1_CH3_Pin;
                break;
            case Channel::CH3N:
                gpio_port = TIM1_CH3N_Port;
                gpio_pin = TIM1_CH3N_Pin;
                break;
            case Channel::CH4:
                gpio_port = TIM1_CH4_Port;
                gpio_pin = TIM1_CH4_Pin;
                break;
        }
        break;
    }

    Gpio io = Gpio(gpio_port, gpio_pin);
    if(en)io.OutAfPP();
    else io.InFloating();
    enable(en);
}
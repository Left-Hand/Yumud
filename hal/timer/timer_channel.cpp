#include "timer_channel.hpp"

#define ADVANCED_TIMER_GPIO_TEMPLATE(x)\
case TIM##x##_BASE:\
    switch(channel){\
        default:\
        case Channel::CH1:\
            gpio_port = &TIM##x##_CH1_Port;\
            gpio_pin = TIM##x##_CH1_Pin;\
            break;\
        case Channel::CH1N:\
            gpio_port = &TIM##x##_CH1N_Port;\
            gpio_pin = TIM##x##_CH1N_Pin;\
            break;\
        case Channel::CH2:\
            gpio_port = &TIM##x##_CH2_Port;\
            gpio_pin = TIM##x##_CH2_Pin;\
            break;\
        case Channel::CH2N:\
            gpio_port = &TIM##x##_CH2N_Port;\
            gpio_pin = TIM##x##_CH2N_Pin;\
            break;\
        case Channel::CH3:\
            gpio_port = &TIM##x##_CH3_Port;\
            gpio_pin = TIM##x##_CH3_Pin;\
            break;\
        case Channel::CH3N:\
            gpio_port = &TIM##x##_CH3N_Port;\
            gpio_pin = TIM##x##_CH3N_Pin;\
            break;\
        case Channel::CH4:\
            gpio_port = &TIM##x##_CH4_Port;\
            gpio_pin = TIM##x##_CH4_Pin;\
            break;\
    }\
    break;\

#define GENERIC_TIMER_GPIO_TEMPLATE(x)\
case TIM##x##_BASE:\
    switch(channel){\
        default:\
        case Channel::CH1:\
            gpio_port = &TIM##x##_CH1_Port;\
            gpio_pin = TIM##x##_CH1_Pin;\
            break;\
        case Channel::CH2:\
            gpio_port = &TIM##x##_CH2_Port;\
            gpio_pin = TIM##x##_CH2_Pin;\
            break;\
        case Channel::CH3:\
            gpio_port = &TIM##x##_CH3_Port;\
            gpio_pin = TIM##x##_CH3_Pin;\
            break;\
        case Channel::CH4:\
            gpio_port = &TIM##x##_CH4_Port;\
            gpio_pin = TIM##x##_CH4_Pin;\
            break;\
    }\
    break;\


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

Gpio & TimerChannel::getPin(){
    Port * gpio_port;
    uint16_t gpio_pin = 0;
    switch((uint32_t)instance){
        default:
        #ifdef HAVE_TIM1
        ADVANCED_TIMER_GPIO_TEMPLATE(1)
        #endif

        #ifdef HAVE_TIM2
        GENERIC_TIMER_GPIO_TEMPLATE(2)
        #endif

        #ifdef HAVE_TIM3
        GENERIC_TIMER_GPIO_TEMPLATE(3)
        #endif

        #ifdef HAVE_TIM4
        GENERIC_TIMER_GPIO_TEMPLATE(4)
        #endif

        #ifdef HAVE_TIM5
        GENERIC_TIMER_GPIO_TEMPLATE(5)
        #endif

        #ifdef HAVE_TIM8
        ADVANCED_TIMER_GPIO_TEMPLATE(8)
        #endif
    }

    return(*gpio_port)[(Pin)gpio_pin];
}
#ifndef __EXTI_HPP__

#define __EXTI_HPP__

#include "src/platform.h"
#include "src/gpio/gpio.hpp"


class Exti{
protected:
    enum class Source:uint8_t{
        PA=0,
        PB=1,
        PC=2,
        PD=3,
        PE=4,
        PF=5
    };

    Source from_gpio_to_source(const Gpio & gpio){
        switch((uint32_t)gpio.instance){
            default:
            #ifdef HAVE_GPIOA
            case GPIOA:
                return Source::PA;
            #endif
            #ifdef HAVE_GPIOB
            case GPIOB:
                return Source::PB;
            #endif
            #ifdef HAVE_GPIOC
            case GPIOC:
                return Source::PC;
            #endif
            #ifdef HAVE_GPIOD
            case GPIOD:
                return Source::PD;
            #endif
            #ifdef HAVE_GPIOE
            case GPIOE:
                return Source::PE;
            #endif
            #ifdef HAVE_GPIOF
            case GPIOF:
                return Source::PF;
            #endif
        }
    }
public:
    Exti(){;}

    void init(){


        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    void bindPin(const Gpio & gpio){
        if(!gpio.isValid()) return;
        EXTI_InitTypeDef EXTI_InitStructure = {0};

        GPIO_EXTILineConfig(from_gpio_to_source(Gpio), gpio.pin_index);
        EXTI_InitStructure.EXTI_Line = EXTI_Line0;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);
    }
    void enableIt(const bool en = true){
        NVIC_InitTypeDef NVIC_InitStructure = {0};
        NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = en;
        NVIC_Init(&NVIC_InitStructure);
    }
};

#endif
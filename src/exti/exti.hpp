#ifndef __EXTI_HPP__

#define __EXTI_HPP__

#include "src/platform.h"
#include "src/nvic/nvic.hpp"
#include "src/gpio/gpio.hpp"

#include <functional>

class Exti{
protected:
    uint8_t pre = 1;
    uint8_t sub = 2;
public:
    enum class Trigger:uint8_t{
        Rising = EXTI_Trigger_Rising,
        Falling = EXTI_Trigger_Falling,
        RisingFalling = EXTI_Trigger_Rising_Falling
    };

    enum class Mode:uint8_t{
        Interrupt = EXTI_Mode_Interrupt,
        Event = EXTI_Mode_Event
    };

    enum class Line:uint32_t{
        _0 = EXTI_Line0,
        _1 = EXTI_Line1,
        _2 = EXTI_Line2,
        _3 = EXTI_Line3,
        _4 = EXTI_Line4,
        _5 = EXTI_Line5,
        _6 = EXTI_Line6,
        _7 = EXTI_Line7,
        _8 = EXTI_Line8,
        _9 = EXTI_Line9,
        _10 = EXTI_Line10,
        _11 = EXTI_Line11,
        _12 = EXTI_Line12,
        _13 = EXTI_Line13,
        _14 = EXTI_Line14,
        _15 = EXTI_Line15,

        Pvd = EXTI_Line16,
        RtcAlarm = EXTI_Line17,
        UsbDWakeup = EXTI_Line18,
        EtherNetWakeUp = EXTI_Line19,
        UsbHSFsWakeUp = EXTI_Line20
        #if defined(CH32V20x_D8) || defined(CH32V20x_D8W)
        ,OscWakeUp = EXTI_Line21

        #endif
    };

protected:
    enum class Source:uint8_t{
        PA=0,
        PB=1,
        PC=2,
        PD=3,
        PE=4,
        PF=5
    };

    Exti(){;}
    Source from_gpio_to_source(const Gpio & gpio){
        switch((uint32_t)gpio.instance){
            default:
            #ifdef HAVE_GPIOA
            case GPIOA_BASE:
                return Source::PA;
            #endif
            #ifdef HAVE_GPIOB
            case GPIOB_BASE:
                return Source::PB;
            #endif
            #ifdef HAVE_GPIOC
            case GPIOC_BASE:
                return Source::PC;
            #endif
            #ifdef HAVE_GPIOD
            case GPIOD_BASE:
                return Source::PD;
            #endif
            #ifdef HAVE_GPIOE
            case GPIOE_BASE:
                return Source::PE;
            #endif
            #ifdef HAVE_GPIOF
            case GPIOF_BASE:
                return Source::PF;
            #endif
        }
    }

    IRQn from_line_to_irqn(const Line & line){
        switch(line){
            case Line::_0:
                return EXTI0_IRQn;
            case Line::_1:
                return EXTI1_IRQn;
            case Line::_2:
                return EXTI2_IRQn;
            case Line::_3:
                return EXTI3_IRQn;
            case Line::_4:
                return EXTI4_IRQn;
            case Line::_5:
            case Line::_6:
            case Line::_7:
            case Line::_8:
            case Line::_9:
                return EXTI9_5_IRQn;
            case Line::_10:
            case Line::_11:
            case Line::_12:
            case Line::_13:
            case Line::_14:
            case Line::_15:
                return EXTI15_10_IRQn;
            default:
                return IRQn(0);
        }
    }
    void enableIt(const IRQn irq, const bool en = true){
        NvicRequest request(irq, pre, sub);
        request.enable(en);
    }
public:
    static Exti & getInstance(){
        static Exti instance;
        return instance;
    }

    void setItPriority(const uint8_t & _pre, const uint8_t & _sub){
        pre = _pre;
        sub = _sub;
    }
    void init(){
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    }

    void bindPin(const Gpio & gpio, const Trigger & trigger = Trigger::RisingFalling, const Mode & mode = Mode::Interrupt){
        if(!gpio.isValid()) return;

        GPIO_EXTILineConfig((uint8_t)from_gpio_to_source(gpio), gpio.getIndex());
        bindLine((Line)(1 << gpio.getIndex()), trigger, mode);
    }

    void bindLine(const Line & line, const Trigger & trigger = Trigger::RisingFalling, const Mode & mode = Mode::Interrupt){
        EXTI_InitTypeDef EXTI_InitStructure = {0};

        EXTI_InitStructure.EXTI_Line = (uint32_t)line;
        EXTI_InitStructure.EXTI_Mode = (EXTIMode_TypeDef)mode;
        EXTI_InitStructure.EXTI_Trigger = (EXTITrigger_TypeDef)trigger;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        if(mode == Mode::Interrupt){
            enableIt(from_line_to_irqn(line), true);
        }
    }

    void bindCb(const Gpio & gpio, const std::function<void(void)> & func);
    void bindCb(const Line & line, const std::function<void(void)> & func);

    static bool getItStatus(const Line & line){
        return EXTI_GetITStatus((uint32_t)line);
    }

    static void clearItStatus(const Line & line){
        EXTI_ClearITPendingBit((uint32_t)line);
    }

    static bool getFlagStatus(const Line & line){
        return EXTI_GetFlagStatus((uint32_t)line);
    }

    static void clearFlagStatus(const Line & line){
        EXTI_ClearFlag((uint32_t)line);
    }
};

extern Exti & exti;

extern "C"{
__interrupt void EXTI0_IRQHandler(void);
__interrupt void EXTI1_IRQHandler(void);
__interrupt void EXTI2_IRQHandler(void);
__interrupt void EXTI3_IRQHandler(void);
__interrupt void EXTI4_IRQHandler(void);
__interrupt void EXTI9_5_IRQHandler(void);
__interrupt void EXTI15_10_IRQHandler(void);
}
#endif
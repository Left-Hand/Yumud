#pragma once

#include "../sys/core/platform.h"
#include "../hal/nvic/nvic.hpp"
#include "../hal/gpio/gpio.hpp"

#include <functional>

class ExtiChannel{
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
        _None = 0,
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

    static Source from_gpio_to_source(const Gpio & gpio){
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

    static Line from_gpio_to_line(const Gpio & gpio){
        return gpio.isValid() ? (Line)(1 << gpio.getIndex()):Line::_None;
    }
    
    static IRQn from_line_to_irqn(const Line & line){
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

    const Line line;

    Gpio * gpio;
    PinMode gpio_mode = PinMode::InPullDN;
    const NvicPriority priority;
    const Trigger trigger;
    const Mode mode;

    friend class CaptureChannelExti;
public:
    ExtiChannel(const Line & _line, const NvicPriority & _priority,
            const Trigger & _trigger = Trigger::Rising, const Mode & _mode = Mode::Interrupt):
            line(_line), gpio(nullptr), priority(_priority), trigger(_trigger), mode(_mode){;}
    ExtiChannel(Gpio & _gpio, const NvicPriority & _priority,
            const Trigger & _trigger = Trigger::Rising,  const Mode & _mode = Mode::Interrupt):
            line(from_gpio_to_line(_gpio)), gpio(&_gpio),priority(_priority), trigger(_trigger),  mode(_mode){
            }

    void setPinMode(const PinMode & _mode){
        gpio_mode = _mode;
        if(gpio) gpio->setMode(_mode);
    }

    void init(){
        if(gpio){
            gpio->setMode(gpio_mode);
            if(gpio->getIndex() > 0) GPIO_EXTILineConfig((uint8_t)from_gpio_to_source(*gpio), gpio->getIndex());
        }

        EXTI_InitTypeDef EXTI_InitStructure = {0};

        EXTI_InitStructure.EXTI_Line = (uint32_t)line;
        EXTI_InitStructure.EXTI_Mode = (EXTIMode_TypeDef)mode;
        EXTI_InitStructure.EXTI_Trigger = (EXTITrigger_TypeDef)trigger;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        if(mode == Mode::Interrupt){
            enableIt(true);
        }
    }


    void bindCb(const std::function<void(void)> & func);

    void enableIt(const bool en = true){
        NvicPriority::enable(priority, from_line_to_irqn(line));
    }
};


extern "C"{
__interrupt void EXTI0_IRQHandler(void);
__interrupt void EXTI1_IRQHandler(void);
__interrupt void EXTI2_IRQHandler(void);
__interrupt void EXTI3_IRQHandler(void);
__interrupt void EXTI4_IRQHandler(void);
__interrupt void EXTI9_5_IRQHandler(void);
__interrupt void EXTI15_10_IRQHandler(void);
}

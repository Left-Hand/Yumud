#pragma once

#include "sys/core/platform.h"
#include "hal/nvic/nvic.hpp"
#include "hal/gpio/gpio.hpp"

#include <functional>

namespace yumud{
class ExtiChannel{
public:
    enum class Trigger:uint8_t{
        Rising = EXTI_Trigger_Rising,
        Falling = EXTI_Trigger_Falling,
        Dual = EXTI_Trigger_Rising_Falling
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
        _15 = EXTI_Line15


        #if defined(CH32)
        ,Pvd = EXTI_Line16
        ,RtcAlarm = EXTI_Line17
        ,UsbDWakeup = EXTI_Line18
        ,EtherNetWakeUp = EXTI_Line19
        ,UsbHSFsWakeUp = EXTI_Line20
        #endif

        #if defined(CH32V20x_D8) || defined(CH32V20x_D8W)
        ,OscWakeUp = EXTI_Line21
        #endif
    };

protected:
    enum class Source:uint8_t{
        PA,PB,PC,PD,PE,PF
    };

    static Source from_gpio_to_source(const Gpio & gpio);
    static Line from_gpio_to_line(const Gpio & gpio);
    static IRQn from_line_to_irqn(const Line line);

    const Line line;

    Gpio * const gpio;
    const PinMode gpio_mode;
    const NvicPriority priority;
    const Trigger trigger;
    const Mode mode;

    friend class CaptureChannelExti;
public:
    ExtiChannel(const Line _line, const NvicPriority & _priority,
            const Trigger _trigger = Trigger::Rising, const Mode _mode = Mode::Interrupt);

    ExtiChannel(Gpio & _gpio, const NvicPriority & _priority,
            const Trigger _trigger = Trigger::Rising,  const Mode _mode = Mode::Interrupt);


    void init();
    void bindCb(std::function<void(void)> && func);
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

}
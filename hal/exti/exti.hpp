#pragma once

#include "hal/gpio/gpio_utils.hpp"

#include "hal/nvic/nvic.hpp"

#include <functional>

namespace ymd::hal{

class Gpio;

enum class ExtiTrigEdge:uint8_t{
    Rising = EXTI_Trigger_Rising,
    Falling = EXTI_Trigger_Falling,
    Dual = EXTI_Trigger_Rising_Falling
};

enum class ExtiTrigMode:uint8_t{
    Interrupt = EXTI_Mode_Interrupt,
    Event = EXTI_Mode_Event
};

enum class ExtiTrigSource:uint32_t{
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

class ExtiChannel final{
public:
    using TrigEdge = ExtiTrigEdge;
    using TrigMode = ExtiTrigMode;
    using TrigSource = ExtiTrigSource;
private:
    const TrigSource source_;

    Gpio * const p_gpio_;
    const GpioMode gpio_mode_;
    const NvicPriority priority_;
    const TrigEdge edge_;
    const TrigMode mode_;

    friend class CaptureChannelExti;
public:
    ExtiChannel(
        const TrigSource line, 
        const NvicPriority priority,
        const TrigEdge edge = TrigEdge::Rising, 
        const TrigMode _mode = TrigMode::Interrupt);

    ExtiChannel(
        Gpio & _gpio, 
        const NvicPriority _priority,
        const TrigEdge edge = TrigEdge::Rising, 
        const TrigMode _mode = TrigMode::Interrupt);


    void init();
    void bind_cb(auto && func){
        // getCallback = std::move(func);
    }


    void enable_it(const bool en = true){
        NvicPriority::enable(
            priority_, 
            map_source_to_irqn(source_)
        );
    }

    static constexpr GpioMode map_edge_to_gpiomode(const TrigEdge edge){
        if(edge == TrigEdge::Dual) return GpioMode::InFloating;
        else if (edge == TrigEdge::Rising) return GpioMode::InPullDN;
        else return GpioMode::InPullUP;
    }

    static constexpr IRQn map_source_to_irqn(const TrigSource source){
            switch(source){
            case TrigSource::_0: return EXTI0_IRQn;
            case TrigSource::_1: return EXTI1_IRQn;
            case TrigSource::_2: return EXTI2_IRQn;
            case TrigSource::_3: return EXTI3_IRQn;
            case TrigSource::_4: return EXTI4_IRQn;
            case TrigSource::_5 ... TrigSource::_9: return EXTI9_5_IRQn;
            case TrigSource::_10 ... TrigSource::_15: return EXTI15_10_IRQn;
            default: return IRQn(0);
        }
    }

    static constexpr TrigSource map_pinsource_to_trigsource(const PinSource source){
        return std::bit_cast<TrigSource>(
            uint32_t(uint16_t(source))
        );
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
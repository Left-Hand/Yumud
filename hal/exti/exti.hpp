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
    _0 = static_cast<uint32_t>(1u << 0),
    _1 = static_cast<uint32_t>(1u << 1),
    _2 = static_cast<uint32_t>(1u << 2),
    _3 = static_cast<uint32_t>(1u << 3),
    _4 = static_cast<uint32_t>(1u << 4),
    _5 = static_cast<uint32_t>(1u << 5),
    _6 = static_cast<uint32_t>(1u << 6),
    _7 = static_cast<uint32_t>(1u << 7),
    _8 = static_cast<uint32_t>(1u << 8),
    _9 = static_cast<uint32_t>(1u << 9),
    _10 = static_cast<uint32_t>(1u << 10),
    _11 = static_cast<uint32_t>(1u << 11),
    _12 = static_cast<uint32_t>(1u << 12),
    _13 = static_cast<uint32_t>(1u << 13),
    _14 = static_cast<uint32_t>(1u << 14),
    _15 = static_cast<uint32_t>(1u << 15)


    #if defined(CH32)
    ,Pvd = static_cast<uint32_t>(1u << 16)
    ,RtcAlarm = static_cast<uint32_t>(1u << 17)
    ,UsbDWakeup = static_cast<uint32_t>(1u << 18)
    ,EtherNetWakeUp = static_cast<uint32_t>(1u << 19)
    ,UsbHSFsWakeUp = static_cast<uint32_t>(1u << 20)
    #endif

    #if defined(CH32V20x_D8) || defined(CH32V20x_D8W)
    ,OscWakeUp = static_cast<uint32_t>(1u << 21)
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


    void enable_it(const Enable en){
        priority_
            .with_irqn(map_source_to_irqn(source_))
            .enable(en);
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

    static constexpr TrigSource map_PinNth_to_trigsource(const PinNth source){
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
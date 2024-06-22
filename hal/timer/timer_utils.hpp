#pragma once

#include "sys/platform.h"
#include "hal/gpio/port.hpp"
#include "hal/nvic/nvic.hpp"
#include <functional>

namespace TimerUtils{
    enum class Mode:uint16_t{
        Up = TIM_CounterMode_Up,
        Down = TIM_CounterMode_Down,
        CenterAlignedDownTrig = TIM_CounterMode_CenterAligned1,
        CenterAlignedUpTrig = TIM_CounterMode_CenterAligned2,
        CenterAlignedDualTrig = TIM_CounterMode_CenterAligned3
    };

    enum class Channel:uint8_t{
        CH1, CH1N, CH2, CH2N, CH3, CH3N, CH4
    };

    enum class IT:uint8_t{
        Update = TIM_IT_Update,
        CC1 = TIM_IT_CC1,
        CC2 = TIM_IT_CC2,
        CC3 = TIM_IT_CC3,
        CC4 = TIM_IT_CC4,
        COM = TIM_IT_COM,
        Trigger = TIM_IT_Trigger,
        Break = TIM_IT_Break,
    };

    bool isBasicTimer(const TIM_TypeDef * instance);

    bool isGenericTimer(const TIM_TypeDef * instance);

    bool isAdvancedTimer(const TIM_TypeDef * instance);

    IRQn ItToIrq(const TIM_TypeDef * instance, const IT & it);

    Gpio & getPin(const TIM_TypeDef * instance, const Channel channel);
};

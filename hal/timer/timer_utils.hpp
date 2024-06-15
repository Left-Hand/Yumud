#pragma once

#include "sys/platform.h"
#include "hal/nvic/nvic.hpp"
#include <functional>

namespace TimerUtils{
    enum class TimerMode:uint16_t{
        Up = TIM_CounterMode_Up,
        Down = TIM_CounterMode_Down,
        CenterAlignedDownTrig = TIM_CounterMode_CenterAligned1,
        CenterAlignedCenterUpTrig = TIM_CounterMode_CenterAligned2,
        CenterAlignedCenterDualTrig = TIM_CounterMode_CenterAligned3
    };

    enum class TimerIT:uint8_t{
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

    IRQn ItToIrq(const TIM_TypeDef * instance, const TimerIT & it);
};

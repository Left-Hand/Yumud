#pragma once

#include "sys/core/system.hpp"
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

    enum class TrgoSource:uint8_t{
        Reset   =  TIM_TRGOSource_Reset,             
        Enable  = TIM_TRGOSource_Enable,           
        Update  = TIM_TRGOSource_Update,           
        OC1     = TIM_TRGOSource_OC1   ,            
        OC1R    = TIM_TRGOSource_OC1Ref,            
        OC2R    = TIM_TRGOSource_OC2Ref,            
        OC3R    = TIM_TRGOSource_OC3Ref,            
        OC4R    = TIM_TRGOSource_OC4Ref            
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

    enum class BdtrLockLevel:uint16_t{
        Off = TIM_LOCKLevel_OFF,
        Low = TIM_LOCKLevel_1,
        Medium = TIM_LOCKLevel_2,
        High = TIM_LOCKLevel_3
    };

    enum class OcMode:uint16_t{
        Timing = TIM_OCMode_Timing,
        Active = TIM_OCMode_Active,
        Inactive = TIM_OCMode_Inactive,
        Toggle = TIM_OCMode_Toggle,
        UpValid = TIM_OCMode_PWM1,
        DownValid = TIM_OCMode_PWM2
    };

    bool isBasicTimer(const TIM_TypeDef * instance);

    bool isGenericTimer(const TIM_TypeDef * instance);

    bool isAdvancedTimer(const TIM_TypeDef * instance);

    IRQn ItToIrq(const TIM_TypeDef * instance, const IT & it);

    Gpio & getPin(const TIM_TypeDef * instance, const Channel channel);
};

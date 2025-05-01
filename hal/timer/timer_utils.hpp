#pragma once

#include "core/platform.hpp"
#include "core/sdk.hpp"

#include <functional>


namespace ymd::hal{
    class Gpio;

    enum class TimerCountMode:uint8_t{
        Up                      = TIM_CounterMode_Up,
        Down                    = TIM_CounterMode_Down,
        CenterAlignedDownTrig   = TIM_CounterMode_CenterAligned1,
        CenterAlignedUpTrig     = TIM_CounterMode_CenterAligned2,
        CenterAlignedDualTrig   = TIM_CounterMode_CenterAligned3
    };

    enum class TimerChannelIndex:uint8_t{
        CH1     =   0b000,
        CH1N    =   0b001,
        CH2     =   0b010,
        CH2N    =   0b011,
        CH3     =   0b100,
        CH3N    =   0b101,
        CH4     =   0b110
    };

    enum class TimerTrgoSource:uint8_t{
        Reset   =  TIM_TRGOSource_Reset,             
        Enable  = TIM_TRGOSource_Enable,           
        Update  = TIM_TRGOSource_Update,           
        OC1     = TIM_TRGOSource_OC1   ,            
        OC1R    = TIM_TRGOSource_OC1Ref,            
        OC2R    = TIM_TRGOSource_OC2Ref,            
        OC3R    = TIM_TRGOSource_OC3Ref,            
        OC4R    = TIM_TRGOSource_OC4Ref            
    };

    enum class TimerIT:uint8_t{
        Update  = TIM_IT_Update,
        CC1     = TIM_IT_CC1,
        CC2     = TIM_IT_CC2,
        CC3     = TIM_IT_CC3,
        CC4     = TIM_IT_CC4,
        COM     = TIM_IT_COM,
        Trigger = TIM_IT_Trigger,
        Break   = TIM_IT_Break,
    };

    enum class TimerBdtrLockLevel:uint16_t{
        Off     = TIM_LOCKLevel_OFF,
        Low     = TIM_LOCKLevel_1,
        Medium  = TIM_LOCKLevel_2,
        High    = TIM_LOCKLevel_3
    };

    enum class TimerOcMode:uint8_t{
        Timing      = TIM_OCMode_Timing,
        Active      = TIM_OCMode_Active,
        Inactive    = TIM_OCMode_Inactive,
        Toggle      = TIM_OCMode_Toggle,
        UpValid     = TIM_OCMode_PWM1,
        DownValid   = TIM_OCMode_PWM2
    };

    namespace internal{
    bool is_basic_timer(const TIM_TypeDef * instance);

    bool is_generic_timer(const TIM_TypeDef * instance);

    bool is_advanced_timer(const TIM_TypeDef * instance);

    IRQn it_to_irq(const TIM_TypeDef * instance, const TimerIT it);

    Gpio & get_pin(const TIM_TypeDef * instance, const TimerChannelIndex channel);
    }

};

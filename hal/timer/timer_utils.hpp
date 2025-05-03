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

    // enum class TimerCountMode:uint8_t{
    //     Up                      = 0x00,
    //     Down                    = 0x01,
    //     CenterAlignedDownTrig   = 0x02,
    //     CenterAlignedUpTrig     = 0x04,
    //     CenterAlignedDualTrig   = 0x06
    // };

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


    // enum class TimerTrgoSource:uint8_t{
    //     Reset   = 0x000,             
    //     Enable  = 0x001,           
    //     Update  = 0x002,           
    //     OC1     = 0x003,            
    //     OC1R    = 0x004,            
    //     OC2R    = 0x005,            
    //     OC3R    = 0x006,            
    //     OC4R    = 0x007            
    // };


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

    // enum class TimerIT:uint8_t{
    //     Update  = 0x01,
    //     CC1     = 0x02,
    //     CC2     = 0x04,
    //     CC3     = 0x08,
    //     CC4     = 0x10,
    //     COM     = 0x20,
    //     Trigger = 0x40,
    //     Break   = 0x80,
    // };

    enum class TimerBdtrLockLevel:uint16_t{
        Off     = TIM_LOCKLevel_OFF,
        Low     = TIM_LOCKLevel_1,
        Medium  = TIM_LOCKLevel_2,
        High    = TIM_LOCKLevel_3
    };

    // enum class TimerBdtrLockLevel:uint8_t{
    //     Off     = 0x00,
    //     Low     = 0x01,
    //     Medium  = 0x02,
    //     High    = 0x03
    // };

    enum class TimerOcMode:uint8_t{
        Freeze              = 0b000,
        ActiveUnlessCvr     = 0b001,
        InactiveUnlessCvr   = 0b010,
        ToggleWhenCvr       = 0b011,
        InactiveForever     = 0b100,
        ActiveForever       = 0b101,
        ActiveBeforeCvr     = 0b110,
        ActiveAfterCvr      = 0b111,
    };

    namespace internal{
    bool is_basic_timer(const TIM_TypeDef * instance);

    bool is_generic_timer(const TIM_TypeDef * instance);

    bool is_advanced_timer(const TIM_TypeDef * instance);

    IRQn it_to_irq(const TIM_TypeDef * instance, const TimerIT it);

    Gpio & get_pin(const TIM_TypeDef * instance, const TimerChannelIndex channel);
    }

};

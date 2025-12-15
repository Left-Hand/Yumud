#pragma once

#include "core/math/real.hpp"

#include <functional>

namespace ymd::hal::adc{

enum class [[nodiscard]] ChannelSelection:uint8_t{
    #if defined(CH32V20X) || defined(CH32V30X) 
    CH0 = 0, 
    CH1, 
    CH2, 
    CH3, 
    CH4, 
    CH5, 
    CH6, 
    CH7, 
    CH8, 
    CH9, 
    CH10, 
    CH11, 
    CH12, 
    CH13, 
    CH14, 
    CH15, 
    TEMP, 
    VREF
    #endif
};

enum class [[nodiscard]] Mode:uint8_t{
    Independent                       	= 0b0000,
    RegInjecSimult                      = 0b0001,
    RegSimult_AlterTrig                	= 0b0010,
    InjecSimult_FastInterl             	= 0b0011,
    InjecSimult_SlowInterl             	= 0b0100,
    InjecSimult                        	= 0b0101,
    RegSimult                          	= 0b0110,
    FastInterl                         	= 0b0111,
    SlowInterl                         	= 0b1000,
    AlterTrig                          	= 0b1001
};

enum class [[nodiscard]] Error:uint8_t{
    None,
    Overrun,
    Watchdog,
    InjectedWatchdog,
    JQOverrun,
    Other
};

enum class [[nodiscard]] SampleCycles:uint8_t{
    #if defined(CH32V20X) || defined(CH32V30X) 
    T1_5   = 0b000,
    T7_5   = 0b001,
    T13_5  = 0b010,
    T28_5  = 0b011,
    T41_5  = 0b100,
    T55_5  = 0b101,
    T71_5  = 0b110,
    T239_5 = 0b111
    #endif

    #if defined(CH32M030) 
    T5_5   = 0b000,
    T11_5   = 0b001,
    T23_5  = 0b010,
    T59_5  = 0b011,
    #endif
};

enum class [[nodiscard]] Pga:uint8_t{
    #if defined(CH32V20X) || defined(CH32V30X) 
    X1 = 0b00, 
    X4 = 0b01, 
    X16 = 0b10, 
    X64 = 0b11
    #endif
};

enum class [[nodiscard]] RegularTrigger:uint8_t{
    #if defined(CH32V20X) || defined(CH32V30X) 
    T1CC1 = 0b000, 
    T1CC2 = 0b001, 
    T1CC3 = 0b010, 
    T2CC2 = 0b011, 
    T3TRGO = 0b100, 
    T4CC4 = 0b101, 
    EXTI11_T8TRGO = 0b110, 
    SW = 0b111
    #endif

    #if defined(CH32M030) 
    SW = 0b0000,
    // nothing 0b0001
    T1CC4 = 0b0010,
    T1CC5 = 0b0011,
    T2CC2 = 0b0100,
    T2CC3 = 0b0101,
    T2CC4 = 0b0110,
    T3CC1 = 0b0111,
    T3CC2 = 0b1000,
    PA14_PB6_EV = 0b1001,
    T1CC4_T1CC5 = 0b1010,
    #endif
};

enum class [[nodiscard]] InjectedTrigger:uint8_t{
    #if defined(CH32V20X) || defined(CH32V30X) 
    T1TRGO = 0b000, 
    T1CC4 = 0b001, 
    T2TRGO = 0b010, 
    T2CC1 = 0b011, 
    T3CC4 = 0b100, 
    T4TRGO = 0b101, 
    EXTI15_T8CC4 = 0b110, 
    SW = 0b111
    #endif

    #if defined(CH32M030)
    T1CC4 = 0b0001,
    T1CC5 = 0b0010,
    T2CC1 = 0b0011,
    T2CC2 = 0b0100,
    T2CC3 = 0b0101,
    T2CC4 = 0b0110,
    T3CC1 = 0b0111,
    T3CC2 = 0b1000,
    T1CC4_T1CC5 = 0b1001,
    PA14_PB6_EV = 0b1010,
    #endif
};


enum class [[nodiscard]] IT:uint16_t{
    EOC = 0x0220,
    JEOC = 0x0480,
    AWD = 0x0140
};

enum class [[nodiscard]] Event:uint8_t{
    EndOfConversion,
    EndOfInjectedConversion,
    AnalogWatchdog
};
}

namespace ymd::hal{
using AdcChannelSelection = adc::ChannelSelection;
using AdcMode = adc::Mode;
using AdcError = adc::Error;
using AdcSampleCycles = adc::SampleCycles;
using AdcPga = adc::Pga;
using AdcRegularTrigger = adc::RegularTrigger;
using AdcInjectedTrigger = adc::InjectedTrigger;
using AdcIT = adc::IT;
using AdcEvent = adc::Event;
}

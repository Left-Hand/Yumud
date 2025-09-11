#pragma once

#include "core/sdk.hpp"

#include "core/math/real.hpp"

#include <functional>

namespace ymd::hal{

enum class AdcChannelNth:uint8_t{
    CH0, CH1, CH2, CH3, 
    CH4, CH5, CH6, CH7, 
    CH8, CH9, CH10, CH11, 
    CH12, CH13, CH14, CH15, 
    TEMP, VREF
};

enum class AdcMode:uint32_t{
    Independent                       	= ADC_Mode_Independent,
    RegInjecSimult                      = ADC_Mode_RegSimult,
    RegSimult_AlterTrig                	= ADC_Mode_RegSimult_AlterTrig,
    InjecSimult_FastInterl             	= ADC_Mode_InjecSimult_FastInterl,
    InjecSimult_SlowInterl             	= ADC_Mode_InjecSimult_SlowInterl,
    InjecSimult                        	= ADC_Mode_InjecSimult,
    RegSimult                          	= ADC_Mode_RegSimult,
    FastInterl                         	= ADC_Mode_FastInterl,
    SlowInterl                         	= ADC_Mode_SlowInterl,
    AlterTrig                          	= ADC_Mode_AlterTrig
};

enum class AdcError:uint8_t{
    None,
    Overrun,
    Watchdog,
    InjectedWatchdog,
    JQOverrun,
    Other
};

enum class AdcSampleCycles:uint8_t{
    T1_5   = 0b000,
    T7_5   = 0b001,
    T13_5  = 0b010,
    T28_5  = 0b011,
    T41_5  = 0b100,
    T55_5  = 0b101,
    T71_5  = 0b110,
    T239_5 = 0b111
};

enum class AdcPga:uint8_t{
    X1, X4, X16, X64
};

enum class AdcRegularTrigger:uint8_t{
    T1CC1, T1CC2, T1CC3, T2CC2, T3TRGO, T4CC4, EXTI11_T8TRGO, SW
};

enum class AdcInjectedTrigger:uint8_t{
    T1TRGO, T1CC4, T2TRGO, T2CC1, T3CC4, T4TRGO, EXTI1515_T8CC4, SW
};


enum class AdcIT:uint16_t{
    EOC = ADC_IT_EOC,
    JEOC = ADC_IT_JEOC,
    AWD = ADC_IT_AWD
};

namespace adc_details{
    void install_pin(const AdcChannelNth channel, const Enable en);
    real_t read_temp();
}

}

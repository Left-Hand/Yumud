#pragma once

#include "hal/analog/adc/adc.hpp"

#if defined(ADC1_PRESENT)

namespace ymd::hal{
class Adc1:public AdcPrimary{
public:
    Adc1();

    #if defined(ADC1_PRESENT) || defined(ADC2_PRESENT)
    friend void ::ADC1_2_IRQHandler(void);
    #endif
};


extern Adc1 adc1;


}

#endif
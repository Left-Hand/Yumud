#pragma once

#include "hal/analog/adc/adc.hpp"

#if defined(ADC1_PRESENT)

namespace ymd::hal{
class Adc1:public AdcPrimary{
public:
    Adc1();
};


extern Adc1 adc1;


}

#endif
#pragma once

#include "hal/adc/adc.hpp"


namespace ymd::hal{
class Adc1:public AdcPrimary{
protected:
    volatile uint8_t regular_conv_index = 0;
    volatile uint8_t injected_conv_index = 0;
    volatile int16_t cali_data;
public:
    Adc1():AdcPrimary(ADC1){;}

    #if defined(ENABLE_ADC1) || defined(ENABLE_ADC2)
    friend void ::ADC1_2_IRQHandler(void);
    #endif
};

#ifdef ENABLE_ADC1
extern Adc1 adc1;
#endif

}
#pragma once

#include "hal/adc/adc.hpp"


namespace ymd::hal{
class Adc1:public AdcPrimary{
protected:
    volatile uint8_t regular_conv_index = 0;
    volatile uint8_t injected_conv_index = 0;
    volatile uint16_t regular_datas[16];
    volatile uint16_t injected_datas[4];
    volatile int16_t cali_data;
public:
    Adc1():AdcPrimary(ADC1){;}

    void refresh_regular_data();
    void refresh_injected_data();

    uint16_t get_regular_data_by_rank(const uint8_t rank);
    uint16_t get_injected_data_by_rank(const uint8_t rank);

    #if defined(ENABLE_ADC1) || defined(ENABLE_ADC2)
    friend void ::ADC1_2_IRQHandler(void);
    #endif
};

#ifdef ENABLE_ADC1
extern Adc1 adc1;
#endif

}
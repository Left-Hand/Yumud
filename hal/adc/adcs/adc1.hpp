#pragma once

#include "../adc.hpp"


namespace ymd{
class Adc1:public AdcPrimary{
protected:
    volatile uint8_t regular_conv_index = 0;
    volatile uint8_t injected_conv_index = 0;
    volatile uint16_t regular_datas[16];
    volatile uint16_t injected_datas[4];
    volatile int16_t cali_data;
public:
    Adc1():AdcPrimary(ADC1){;}

    void refreshRegularData() override;
    void refreshInjectedData() override;

    uint16_t getRegularDataByRank(const uint8_t rank) override;
    uint16_t getInjectedDataByRank(const uint8_t rank) override;

    #if defined(ENABLE_ADC1) || defined(ENABLE_ADC2)
    friend void ::ADC1_2_IRQHandler(void);
    #endif
};

#ifdef ENABLE_ADC1
extern Adc1 adc1;
#endif

}
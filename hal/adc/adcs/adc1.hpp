#ifndef __ADC1_HPP__

#define __ADC1_HPP__

#include "../adc.hpp"

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

};

#ifdef HAVE_ADC1
extern Adc1 adc1;
#endif




#endif
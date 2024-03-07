#ifndef __ADC1_HPP__

#define __ADC1_HPP__

#include "../adc.hpp"

class Adc1:public AdcPrimary{
public:
    Adc1():AdcPrimary(ADC1){;}
};

#define HAVE_ADC1

#ifdef HAVE_ADC1
extern Adc1 adc1;
#endif


extern "C"{
__interrupt void ADC1_2_IRQHandler(void);

}
#endif
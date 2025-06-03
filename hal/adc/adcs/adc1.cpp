#include "adc1.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::hal;

#if defined(ENABLE_ADC1) || defined(ENABLE_ADC2)
__interrupt void ADC1_2_IRQHandler(void){
    // const uint32_t tempCTLR = ADC1->CTLR1;
    const uint32_t tempSTATR = ADC1->STATR;

    // #define CHECK_IT(x) (tempSTATR & (x >> 8) and tempCTLR & (x & 0xFF))
    #define CHECK_IT(x) (tempSTATR & (x >> 8))
    #define CLEAR_IT(x) ADC1->STATR = ~(uint32_t)(x >> 8);

    if(CHECK_IT(ADC_IT_JEOC)){
        adc1.on_jeoc_interrupt();
        CLEAR_IT(ADC_IT_JEOC);
    }else if(CHECK_IT(ADC_IT_EOC)){
        adc1.on_eoc_interrupt();
        CLEAR_IT(ADC_IT_EOC);
    }else if(CHECK_IT(ADC_IT_AWD)){
        adc1.on_awd_interrupt();
        CLEAR_IT(ADC_IT_AWD);
    }
}
#endif

namespace ymd::hal{
#ifdef ENABLE_ADC1
Adc1 adc1;
#endif
}
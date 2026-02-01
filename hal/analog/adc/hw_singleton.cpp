#include "hw_singleton.hpp"
#include "core/debug/debug.hpp"
#include "core/sdk.hpp"

using namespace ymd::hal;


#if defined(ADC1_PRESENT)
Adc1::Adc1():AdcPrimary(ADC1){;}
#endif

void AdcInterruptDispatcher::on_interrupt(){
    // const uint32_t tempCTLR = ADC1->CTLR1;
    const uint32_t temp_statr = ADC1->STATR;

    // #define CHECK_IT(x) (temp_statr & (x >> 8) and tempCTLR & (x & 0xFF))
    #define CHECK_IT(x) (temp_statr & (x >> 8))
    #define CLEAR_IT(x) ADC1->STATR = ~(uint32_t)(x >> 8);

    if(CHECK_IT(ADC_IT_JEOC)){
        adc1.isr_jeoc();
        CLEAR_IT(ADC_IT_JEOC);
    }else if(CHECK_IT(ADC_IT_EOC)){
        adc1.isr_eoc();
        CLEAR_IT(ADC_IT_EOC);
    }else if(CHECK_IT(ADC_IT_AWD)){
        adc1.isr_awd();
        CLEAR_IT(ADC_IT_AWD);
    }
}

#if defined(ADC1_PRESENT) || defined(ADC2_PRESENT)
__interrupt void ADC1_2_IRQHandler(void){
    AdcInterruptDispatcher::on_interrupt();
}
#endif

namespace ymd::hal{
#ifdef ADC1_PRESENT
Adc1 adc1;
#endif
}
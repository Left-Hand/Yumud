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


void Adc1::refresh_regular_data(){
    if(regular_conv_index < 16){
        regular_datas[regular_conv_index] = ADC1->RDATAR;
        regular_conv_index++;
    }
}

void Adc1::refresh_injected_data(){
    injected_datas[0] = instance->IDATAR1;
    injected_datas[1] = instance->IDATAR2;
    injected_datas[2] = instance->IDATAR3;
    injected_datas[3] = instance->IDATAR4;
}

uint16_t Adc1::get_regular_data_by_rank(const uint8_t rank){
    return 0;
}

uint16_t Adc1::get_injected_data_by_rank(const uint8_t rank){
    if(rank == 0 || rank > 4) return 0;
    else return injected_datas[rank];
}

namespace ymd::hal{
#ifdef ENABLE_ADC1
Adc1 adc1;
#endif
}
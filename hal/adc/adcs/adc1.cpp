#include "adc1.hpp"

#ifdef HAVE_ADC1
Adc1 adc1;
#endif

#if defined(HAVE_ADC1) || defined(HAVE_ADC2)

__interrupt void ADC1_2_IRQHandler(void){
    if(ADC_GetITStatus(ADC1,ADC_IT_JEOC)){


        ADC_ClearITPendingBit(ADC1,ADC_IT_JEOC);
    }else if(ADC_GetITStatus(ADC1, ADC_IT_EOC)){
        

        ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
    }else if(ADC_GetITStatus(ADC1,ADC_IT_AWD)){


        ADC_ClearITPendingBit(ADC1,ADC_IT_AWD);
    }
}

#endif

static volatile uint8_t regular_conv_index = 0;
static volatile uint8_t injected_conv_index = 0;
static volatile uint16_t regular_datas[16];
static volatile uint16_t injected_datas[4];
static volatile int16_t cali_data;


void Adc1::refreshRegularData(){
    if(regular_conv_index < 16){
        regular_datas[regular_conv_index] = ADC1->RDATAR;
        regular_conv_index++;
    }
}

void Adc1::refreshInjectedData(){
    injected_datas[0] = instance->IDATAR1;
    injected_datas[1] = instance->IDATAR2;
    injected_datas[2] = instance->IDATAR3;
    injected_datas[3] = instance->IDATAR4;
}

uint16_t Adc1::getRegularDataByRank(const uint8_t & rank){
    return 0;
}

uint16_t Adc1::getInjectedDataByRank(const uint8_t & rank){
    if(rank == 0 || rank > 4) return 0;
    else return injected_datas[rank];
}


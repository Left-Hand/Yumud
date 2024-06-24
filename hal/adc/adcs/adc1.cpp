#include "adc1.hpp"

#ifdef HAVE_ADC1
Adc1 adc1;
#endif

#if defined(HAVE_ADC1) || defined(HAVE_ADC2)
using Callback = AdcUtils::Callback;

static Callback jeoc_cb;
static Callback eoc_cb;
static Callback awd_cb;

__interrupt void ADC1_2_IRQHandler(void){
    if(ADC_GetITStatus(ADC1,ADC_IT_JEOC)){
        EXECUTE(jeoc_cb);
        ADC_ClearITPendingBit(ADC1,ADC_IT_JEOC);
    }else if(ADC_GetITStatus(ADC1, ADC_IT_EOC)){
        EXECUTE(eoc_cb);
        ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
    }else if(ADC_GetITStatus(ADC1,ADC_IT_AWD)){
        EXECUTE(awd_cb);
        ADC_ClearITPendingBit(ADC1,ADC_IT_AWD);
    }
}

#endif

static volatile uint8_t regular_conv_index = 0;
static volatile uint8_t injected_conv_index = 0;
static volatile uint16_t regular_datas[16];
static volatile uint16_t injected_datas[4];
static volatile int16_t cali_data;

void AdcPrimary::bindCb(const IT it,Callback && cb){
    switch(it){
        case IT::JEOC:
            jeoc_cb = cb;
            break;
        case IT::EOC:
            eoc_cb = cb;
            break;
        case IT::AWD:
            awd_cb = cb;
            break;
        default:
            break;
    }
}


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


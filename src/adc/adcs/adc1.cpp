#include "adc1.hpp"

__interrupt void ADC1_2_IRQHandler(void){

    if(ADC_GetITStatus(ADC1,ADC_IT_JEOC)){


        ADC_ClearITPendingBit(ADC1,ADC_IT_JEOC);
    }else if(ADC_GetITStatus(ADC1, ADC_IT_AWD)){


        ADC_ClearITPendingBit(ADC1,ADC_IT_AWD);
    }else if(ADC_GetITStatus(ADC1,ADC_IT_EOC)){


        ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
    }
}

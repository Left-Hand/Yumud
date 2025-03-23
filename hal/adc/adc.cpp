#include "adc.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::hal;

void AdcPrimary::init(const std::initializer_list<AdcChannelConfig> & regular_list,
        const std::initializer_list<AdcChannelConfig> & injected_list, const AdcMode mode){

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_DeInit(instance);

    ADC_InitTypeDef ADC_InitStructure; 
	
    ADC_DeInit(instance);
	
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_OutputBuffer = ADC_OutputBuffer_Disable;
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    ADC_Init(instance, &ADC_InitStructure);

    bool temp_verf_activation = false;

    { 
        set_regular_count(regular_list.size());
        uint8_t i = 0;
        for(auto config : regular_list){
            i++;
            ADC_RegularChannelConfig(instance,(uint8_t)config.channel,i,(uint8_t)config.cycles);
            __adc_internal::install_pin(config.channel);

            temp_verf_activation |= (config.channel == ChannelIndex::TEMP || config.channel == ChannelIndex::VREF);

            if(i > 16) break;
        }
    }

    {
        set_injected_count(injected_list.size());
        uint8_t i = 0;
        for(auto config : injected_list){
            i++;

            ADC_InjectedChannelConfig(instance,(uint8_t)config.channel,i,(uint8_t)config.cycles);
            ADC_SetInjectedOffset(instance, ADC_InjectedChannel_1 + (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i-1),MAX(cali_data, 0)); // offset can`t be negative
            __adc_internal::install_pin(config.channel);

            temp_verf_activation |= (
                config.channel == ChannelIndex::TEMP || 
                config.channel == ChannelIndex::VREF
            );

            if(i > 4) break;
        }
    }

    if(temp_verf_activation) enable_temp_vref(true);

    if(MAX(injected_list.size(), regular_list.size()) > 1){
        enable_scan(true);  
    }else{
        enable_singleshot(true);
    }


    ADC_ExternalTrigConvCmd(instance, ENABLE);
    ADC_ExternalTrigInjectedConvCmd(instance, ENABLE);

    ADC_DMACmd(instance, DISABLE);
    
    ADC_ClearITPendingBit(instance, ADC_IT_JEOC | ADC_IT_AWD | ADC_IT_EOC);
    
    ADC_AutoInjectedConvCmd(instance, ENABLE);

    ADC_Cmd(instance, ENABLE);

    {
        ADC_BufferCmd(instance, DISABLE);
        ADC_ResetCalibration(instance);
        while(ADC_GetResetCalibrationStatus(instance));
        ADC_StartCalibration(instance);
        while(ADC_GetCalibrationStatus(instance));
        cali_data = Get_CalibrationValue(instance);
    }

    ADC_BufferCmd(instance, ENABLE);
}


AdcInjectedChannel & AdcPrimary::inj(const size_t index){
    if(index == 0 or index > 4) PANIC();
    return injected_channels[index - 1];
}
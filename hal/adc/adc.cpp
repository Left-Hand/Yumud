#include "adc.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::hal;

void AdcPrimary::init(
        const std::initializer_list<AdcChannelConfig> & regular_list,
        const std::initializer_list<AdcChannelConfig> & injected_list, 
        const AdcMode mode
){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);	
    ADC_DeInit(instance_);
    ADC_InitTypeDef ADC_InitStructure; 
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_OutputBuffer = ADC_OutputBuffer_Disable;
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    ADC_Init(instance_, &ADC_InitStructure);

    bool temp_verf_activation = false;

    auto channel_is_temp_or_vref = [](const ChannelIndex channel_index){
        return channel_index == ChannelIndex::TEMP or
                channel_index == ChannelIndex::VREF;
    };

    { 
        set_regular_count(regular_list.size());
        uint8_t i = 0;
        for(const auto & cfg : regular_list){
            i++;
            ADC_RegularChannelConfig(instance_,(uint8_t)cfg.channel,i,(uint8_t)cfg.cycles);
            adc_details::install_pin(cfg.channel);

            temp_verf_activation |= channel_is_temp_or_vref(cfg.channel);

            if(i > 16) break;
        }
    }

    {
        set_injected_count(injected_list.size());
        uint8_t i = 0;
        for(const auto & cfg : injected_list){
            i++;

            ADC_InjectedChannelConfig(instance_,(uint8_t)cfg.channel,i,(uint8_t)cfg.cycles);
            ADC_SetInjectedOffset(instance_, ADC_InjectedChannel_1 + 
                (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i-1),MAX(cali_data, 0)); 
                // offset can`t be negative
            adc_details::install_pin(cfg.channel);

            temp_verf_activation |= (
                cfg.channel == ChannelIndex::TEMP || 
                cfg.channel == ChannelIndex::VREF
            );

            if(i > 4) break;
        }
    }

    if(temp_verf_activation) enable_temp_vref(EN);

    if(MAX(injected_list.size(), regular_list.size()) > 1){
        enable_scan(EN);  
    }else{
        enable_singleshot(EN);
    }


    ADC_ExternalTrigConvCmd(instance_, ENABLE);
    ADC_ExternalTrigInjectedConvCmd(instance_, ENABLE);

    ADC_DMACmd(instance_, DISABLE);
    
    ADC_ClearITPendingBit(instance_, ADC_IT_JEOC | ADC_IT_AWD | ADC_IT_EOC);
    
    ADC_AutoInjectedConvCmd(instance_, ENABLE);

    ADC_Cmd(instance_, ENABLE);

    {
        ADC_BufferCmd(instance_, DISABLE);
        ADC_ResetCalibration(instance_);
        while(ADC_GetResetCalibrationStatus(instance_));
        ADC_StartCalibration(instance_);
        while(ADC_GetCalibrationStatus(instance_));
        cali_data = Get_CalibrationValue(instance_);
    }

    ADC_BufferCmd(instance_, ENABLE);
}

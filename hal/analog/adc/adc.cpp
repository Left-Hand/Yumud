#include "adc.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::hal;

void AdcPrimary::set_regular_channels(
    const std::initializer_list<AdcChannelConfig> & regular_list
){ 
    set_regular_count(regular_list.size());
    uint8_t i = 0;
    for(const auto & regular_cfg : regular_list){
        i++;
        ADC_RegularChannelConfig(
            inst_,
            static_cast<uint8_t>(regular_cfg.nth),
            i,
            static_cast<uint8_t>(regular_cfg.cycles)
        );
        adc_details::install_pin(regular_cfg.nth, EN);

        if(i > 16) break;
    }
}


void AdcPrimary::set_injected_channels(
    const std::initializer_list<AdcChannelConfig> & injected_list
){
    {
        set_injected_count(injected_list.size());
        uint8_t i = 0;
        for(const auto & injected_cfg : injected_list){
            i++;

            ADC_InjectedChannelConfig(
                inst_,
                static_cast<uint8_t>(injected_cfg.nth),
                i,
                static_cast<uint8_t>(injected_cfg.cycles));

            ADC_SetInjectedOffset(
                inst_, ADC_InjectedChannel_1 + 
                (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i-1),MAX(cali_data, 0)); 
                // offset can`t be negative
            adc_details::install_pin(injected_cfg.nth, EN);

            if(i > 4) break;
        }
    }
}
void AdcPrimary::init(
    const std::initializer_list<AdcChannelConfig> & regular_list,
    const std::initializer_list<AdcChannelConfig> & injected_list, 
    const Config & cfg
){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);	
    ADC_DeInit(inst_);

    const ADC_InitTypeDef ADC_InitStructure = {
        .ADC_Mode = ADC_Mode_Independent,
        .ADC_ScanConvMode = DISABLE,
        .ADC_ContinuousConvMode = DISABLE,
        .ADC_ExternalTrigConv = ADC_ExternalTrigConv_None,
        .ADC_DataAlign = ADC_DataAlign_Right,
        .ADC_NbrOfChannel = 1,
        .ADC_OutputBuffer = ADC_OutputBuffer_Disable,
        .ADC_Pga = ADC_Pga_1,
    };

    ADC_Init(inst_, &ADC_InitStructure);

    bool temp_verf_activation = [&]{
        auto channel_is_temp_or_vref = [](const ChannelNth nth){
            return nth == ChannelNth::TEMP or
                    nth == ChannelNth::VREF;
        };
        for(const auto cfg : injected_list){
            if(channel_is_temp_or_vref(cfg.nth))
                return true;
        }
        for(const auto cfg : regular_list){
            if(channel_is_temp_or_vref(cfg.nth))
                return true;
        }
        return false;
    }();


    set_regular_channels(regular_list);
    set_injected_channels(injected_list);

    if(temp_verf_activation) enable_temp_vref(EN);

    if(MAX(injected_list.size(), regular_list.size()) > 1){
        enable_scan(EN);  
    }else{
        enable_singleshot(EN);
    }


    ADC_ExternalTrigConvCmd(inst_, ENABLE);
    ADC_ExternalTrigInjectedConvCmd(inst_, ENABLE);

    ADC_DMACmd(inst_, DISABLE);
    
    ADC_ClearITPendingBit(inst_, ADC_IT_JEOC | ADC_IT_AWD | ADC_IT_EOC);
    
    ADC_AutoInjectedConvCmd(inst_, ENABLE);

    ADC_Cmd(inst_, ENABLE);

    {
        ADC_BufferCmd(inst_, DISABLE);
        ADC_ResetCalibration(inst_);
        while(ADC_GetResetCalibrationStatus(inst_));
        ADC_StartCalibration(inst_);
        while(ADC_GetCalibrationStatus(inst_));
        cali_data = Get_CalibrationValue(inst_);
    }

    ADC_BufferCmd(inst_, ENABLE);
}

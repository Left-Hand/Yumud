#include "adc.hpp"

void AdcPrimary::init(const std::initializer_list<AdcChannelConfig> regular_list,
        const std::initializer_list<AdcChannelConfig> injected_list, const Mode mode){

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_DeInit(instance);

    // setMode(mode);
    // setRegularTrigger(RegularTrigger::SW);
    // setInjectedTrigger(InjectedTrigger::SW);

    ADC_InitTypeDef ADC_InitStructure; 
	
    ADC_DeInit(ADC1);  // 将外设ADC1的全部寄存器重设为缺省值
	
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                   // ADC工作模式:ADC1和ADC2工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                        // 模数转换工作在单通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                  // 模数转换工作在单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // 转换由软件而不是外部触发启动
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               // ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                              // 顺序进行规则转换的ADC通道的数目
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    ADC_Init(ADC1, &ADC_InitStructure);    // 根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器


    ADC_Cmd(instance, ENABLE);

    ADC_BufferCmd(instance, DISABLE);

    ADC_ResetCalibration(instance);

    while(ADC_GetResetCalibrationStatus(instance));
    ADC_StartCalibration(instance);

    while(ADC_GetCalibrationStatus(instance));
    cali_data = Get_CalibrationValue(instance);

    ADC_BufferCmd(instance, ENABLE);

    bool temp_verf_activation = false;

    {
        setRegularCount(regular_list.size());
        uint8_t i = 0;
        for(auto config : regular_list){
            i++;
            ADC_RegularChannelConfig(instance,(uint8_t)config.channel,i,(uint8_t)config.cycles);
            AdcUtils::installPin(config.channel);

            temp_verf_activation |= (config.channel == Channel::TEMP || config.channel == Channel::VREF);

            if(i > 16) break;
        }
    }

    {
        setInjectedCount(injected_list.size());
        uint8_t i = 0;
        for(auto config : injected_list){
            i++;

            ADC_InjectedChannelConfig(instance,(uint8_t)config.channel,i,(uint8_t)config.cycles);
            ADC_SetInjectedOffset(instance, ADC_InjectedChannel_1 + (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i-1),MAX(cali_data, 0)); // offset can`t be negative
            installPin(config.channel);

            temp_verf_activation |= (config.channel == Channel::TEMP || config.channel == Channel::VREF);

            if(i > 4) break;
        }
    }

    if(temp_verf_activation) enableTempVref();
    if(MAX(injected_list.size(), regular_list.size()) > 1) enableScan();
    else enableSingleshot();

}
#include "tb.h"

void adc_Init(void)

{

    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1, ENABLE );



    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;

    ADC_InitStructure.ADC_ScanConvMode = DISABLE;

    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;

    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;

    ADC_InitStructure.ADC_NbrOfChannel = 1;

    ADC_Init(ADC1, &ADC_InitStructure);



    ADC_TempSensorVrefintCmd(ENABLE);



    ADC_Cmd(ADC1, ENABLE);



    ADC_ResetCalibration(ADC1);



    while(ADC_GetResetCalibrationStatus(ADC1));



    ADC_StartCalibration(ADC1);



    while(ADC_GetCalibrationStatus(ADC1));

}



u16 get_adc(u8 ch)

{

    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );



    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));

    return ADC_GetConversionValue(ADC1);

}



u16 get_adc_average(u8 ch,u8 times)

{

    u32 temp_val=0;

    u8 t;

    for(t=0;t<times;t++)

    {

        temp_val+=get_adc(ch);

        delay(5);

    }

    return temp_val/times;

}



float get_temperature(void)

{

    u32 adcx;

    float temperate;

    adcx=get_adc_average(ADC_Channel_16,20);

    temperate=(float)adcx*(3.3/4096);

    temperate=(1.43-temperate)/0.0043+25;

    return temperate;

}

void temp_tb2(OutputStream & logger){
    {
    adc_Init();           
    while(1)
    {
        logger.println(get_temperature());
        delay(200);    
    } 
    }
}
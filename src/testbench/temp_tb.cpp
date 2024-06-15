#include "tb.h"   

// ��ʼ������ADC�������Թ���ͨ��Ϊ����
void T_Adc_Init(void)  
{
    ADC_InitTypeDef ADC_InitStructure; 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE);      // ʹ��ADC1ͨ��ʱ��
  
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   // ��Ƶ����6ʱ��Ϊ72M/6=12MHz
	
    ADC_DeInit(ADC1);  // ������ADC1��ȫ���Ĵ�������Ϊȱʡֵ
	
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                   // ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                        // ģ��ת�������ڵ�ͨ��ģʽ
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                  // ģ��ת�������ڵ���ת��ģʽ
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // ת��������������ⲿ��������
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               // ADC�����Ҷ���
    ADC_InitStructure.ADC_NbrOfChannel = 1;                              // ˳����й���ת����ADCͨ������Ŀ
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    ADC_Init(ADC1, &ADC_InitStructure);    // ����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);    // ADC1����ͨ��ת��,����ʱ��Ϊ239.5���� 
 
    ADC_Cmd(ADC1, ENABLE);    // ʹ��ָ����ADC1
	ADC_TempSensorVrefintCmd(ENABLE);  // �����ڲ��¶ȴ�����
	
    ADC_ResetCalibration(ADC1);     // ����ָ����ADC1�ĸ�λ�Ĵ���
    while(ADC_GetResetCalibrationStatus(ADC1));    // ��ȡADC1����У׼�Ĵ�����״̬,����״̬��ȴ�

    ADC_StartCalibration(ADC1);     // ADC1У׼
    while(ADC_GetCalibrationStatus(ADC1));        // ��ȡָ��ADC1��У׼����,����״̬��ȴ�

	// ADC_SoftwareStartConvCmd(ADC1,ENABLE); // ���������ʼ
}

// ��ȡADCֵ
uint16_t T_Get_Adc(uint8_t ch)   
{
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);    // ADC1����ͨ��ת��,����ʱ��Ϊ239.5����                      
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);         // ʹ��ָ����ADC1�����ת����������
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));  // �ȴ�ת������
    return ADC_GetConversionValue(ADC1);            // �������һ��ADC1�������ת�����
}

// ��ȡͨ��ch��ת��ֵ��ȡtimes��ƽ��ֵ��
uint16_t T_Get_Adc_Average(uint8_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;
    for(t = 0; t < times; t++)
    {
        temp_val += T_Get_Adc(ch);
        delay(5);
    }
    return temp_val / times;
}        

// ��ȡ�ڲ��¶ȴ������¶�ֵ
// ����ֵ:�¶�ֵ(������100������λ:��)
int Get_Temprate(void)
{
    uint32_t adcx;
    adcx = T_Get_Adc_Average(ADC_Channel_16, 20); // ��ȡͨ��16, 20��ȡƽ��

        s32 val_mv = (adcx * 3300 / 4096);

    return TempSensor_Volt_To_Temper(val_mv);
}



void temp_tb(OutputStream & logger){
    {
    T_Adc_Init();            // ADC��ʼ��              
    while(1)
    {
        logger.println(Get_Temprate());
        delay(200);    
    } 
    }
}
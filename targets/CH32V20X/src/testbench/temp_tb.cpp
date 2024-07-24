#include "tb.h"   

// 初始化配置ADC参数（以规则通道为例）
void T_Adc_Init(void)  
{
    ADC_InitTypeDef ADC_InitStructure; 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE);      // 使能ADC1通道时钟
  
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   // 分频因子6时钟为72M/6=12MHz
	
    ADC_DeInit(ADC1);  // 将外设ADC1的全部寄存器重设为缺省值
	
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                   // ADC工作模式:ADC1和ADC2工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                        // 模数转换工作在单通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                  // 模数转换工作在单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // 转换由软件而不是外部触发启动
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               // ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                              // 顺序进行规则转换的ADC通道的数目
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    ADC_Init(ADC1, &ADC_InitStructure);    // 根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);    // ADC1规则通道转换,采样时间为239.5周期 
 
    ADC_Cmd(ADC1, ENABLE);    // 使能指定的ADC1
	ADC_TempSensorVrefintCmd(ENABLE);  // 开启内部温度传感器
	
    ADC_ResetCalibration(ADC1);     // 重置指定的ADC1的复位寄存器
    while(ADC_GetResetCalibrationStatus(ADC1));    // 获取ADC1重置校准寄存器的状态,设置状态则等待

    ADC_StartCalibration(ADC1);     // ADC1校准
    while(ADC_GetCalibrationStatus(ADC1));        // 获取指定ADC1的校准程序,设置状态则等待

	// ADC_SoftwareStartConvCmd(ADC1,ENABLE); // 软件触发开始
}

// 获取ADC值
uint16_t T_Get_Adc(uint8_t ch)   
{
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);    // ADC1规则通道转换,采样时间为239.5周期                      
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);         // 使能指定的ADC1的软件转换启动功能
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));  // 等待转换结束
    return ADC_GetConversionValue(ADC1);            // 返回最近一次ADC1规则组的转换结果
}

// 获取通道ch的转换值（取times次平均值）
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

// 获取内部温度传感器温度值
// 返回值:温度值(扩大了100倍，单位:℃)
int Get_Temprate(void)
{
    uint32_t adcx;
    adcx = T_Get_Adc_Average(ADC_Channel_16, 20); // 读取通道16, 20次取平均

        s32 val_mv = (adcx * 3300 / 4096);

    return TempSensor_Volt_To_Temper(val_mv);
}



void temp_tb(OutputStream & logger){
    {
    T_Adc_Init();            // ADC初始化              
    while(1)
    {
        logger.println(Get_Temprate());
        delay(200);    
    } 
    }
}
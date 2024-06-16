#pragma once

enum class AdcChannel:uint8_t{
    CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, TEMP, VREF
};


enum class AdcMode:uint32_t{
    Independent                       	= ADC_Mode_Independent,
    RegInjecSimult                      = ADC_Mode_RegSimult,
	RegSimult_AlterTrig                	= ADC_Mode_RegSimult_AlterTrig,
	InjecSimult_FastInterl             	= ADC_Mode_InjecSimult_FastInterl,
	InjecSimult_SlowInterl             	= ADC_Mode_InjecSimult_SlowInterl,
	InjecSimult                        	= ADC_Mode_InjecSimult,
	RegSimult                          	= ADC_Mode_RegSimult,
	FastInterl                         	= ADC_Mode_FastInterl,
	SlowInterl                         	= ADC_Mode_SlowInterl,
	AlterTrig                          	= ADC_Mode_AlterTrig
};


enum class AdcSampleCycles:uint8_t{
    T1_5 = ADC_SampleTime_1Cycles5,
    T7_5 = ADC_SampleTime_7Cycles5,
    T13_5 = ADC_SampleTime_13Cycles5,
    T28_5 = ADC_SampleTime_28Cycles5,
    T41_5 = ADC_SampleTime_41Cycles5,
    T55_5 = ADC_SampleTime_55Cycles5,
    T71_5 = ADC_SampleTime_71Cycles5,
    T239_5 = ADC_SampleTime_239Cycles5
};

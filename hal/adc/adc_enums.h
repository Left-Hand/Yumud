#ifndef __ADC_ENUMS_H__

#define __ADC_ENUMS_H__

enum class AdcChannels:uint8_t{
    CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, TEMP, VREF
};


enum class AdcMode:uint32_t{
    Independent = ((uint32_t)0x00000000),
    RegInjecSimult                        =((uint32_t)0x00010000),
    RegSimult_AlterTrig                   =((uint32_t)0x00020000),
    InjecSimult_FastInterl                =((uint32_t)0x00030000),
    InjecSimult_SlowInterl                =((uint32_t)0x00040000),
    InjecSimult                           =((uint32_t)0x00050000),
    RegSimult                             =((uint32_t)0x00060000),
    FastInterl                            =((uint32_t)0x00070000),
    SlowInterl                            =((uint32_t)0x00080000),
    AlterTrig                             =((uint32_t)0x00090000)
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

#endif
#ifndef __ADC_ENUMS_H__

#define __ADC_ENUMS_H__

enum class AdcChannels:uint8_t{
    CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, TEMP, VREF
};

enum class AdcSampleTimes:uint8_t{
    T1_5 = ADC_SampleTime_1Cycles5,
    T7_5 = ADC_SampleTime_7Cycles5,
    T13_5 = ADC_SampleTime_13Cycles5,
    T28_5 = ADC_SampleTime_28Cycles5,
    T41_5 = ADC_SampleTime_41Cycles5,
    T55_5 = ADC_SampleTime_55Cycles5,
    T71_5 = ADC_SampleTime_71Cycles5
};

#endif
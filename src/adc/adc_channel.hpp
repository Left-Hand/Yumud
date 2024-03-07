#ifndef __ADC_CHANNEL_HPP__

#define __ADC_CHANNEL_HPP__

#include "src/platform.h"

class AdcChannel{
public:

public:

};

class AdcChannelOnChip: public AdcChannel{
public:
    enum class Channel:uint8_t{
        CH0, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8, CH9, CH10, CH11, CH12, CH13, CH14, CH15, TEMP, VREF
    };

    enum class SampleTime:uint8_t{
        T1_5 = ADC_SampleTime_1Cycles5,
        T7_5 = ADC_SampleTime_7Cycles5,
        T13_5 = ADC_SampleTime_13Cycles5,
        T28_5 = ADC_SampleTime_28Cycles5,
        T41_5 = ADC_SampleTime_41Cycles5,
        T55_5 = ADC_SampleTime_55Cycles5,
        T71_5 = ADC_SampleTime_71Cycles5
    };
protected:
    ADC_TypeDef * instance;
    Channel channel;
    SampleTime sample_time;

public:
    AdcChannelOnChip(ADC_TypeDef * _instance,
            const Channel _channel = Channel::CH0, const SampleTime _sample_time = SampleTime::T28_5):
            instance(_instance), channel(_channel),
            sample_time(_sample_time){;}

    virtual void setSampleTime(const SampleTime time) = 0;
    virtual void setCaliData(const uint16_t data) = 0;
    virtual void installToPin() = 0;
};


#endif



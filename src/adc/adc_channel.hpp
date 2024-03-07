#ifndef __ADC_CHANNEL_HPP__

#define __ADC_CHANNEL_HPP__

#include "src/platform.h"
#include "analog_channel.hpp"
#include "adc_enums.h"
#include "src/gpio/gpio.hpp"

class Adc;
class AdcPrimary;
class AdcCompanion;

class AdcChannelOnChip{

protected:
    using Channel = AdcChannels;
    using SampleTime = AdcSampleTimes;

    ADC_TypeDef * instance;
    Channel channel;
    SampleTime sample_time;

    virtual void setCaliData(const uint16_t data) = 0;

    friend class Adc;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    AdcChannelOnChip(ADC_TypeDef * _instance,
            const Channel _channel = Channel::CH0, const SampleTime _sample_time = SampleTime::T28_5):
            instance(_instance), channel(_channel),
            sample_time(_sample_time){;}

    virtual void setSampleTime(const SampleTime time) = 0;

    void installToPin(const bool en = true);

    void init(){
        installToPin();
    }
};


#endif



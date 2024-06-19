#ifndef __ADC_CHANNEL_HPP__

#define __ADC_CHANNEL_HPP__

#include "sys/platform.h"
#include "adc_enums.h"

#include "analog_channel.hpp"


class AdcOnChip;
class AdcPrimary;
class AdcCompanion;


class AdcChannelConcept:public AnalogInChannel{

};

class AdcChannelOnChip: public AdcChannelConcept{
protected:
    using Channel = AdcUtils::Channel;
    using SampleCycles = AdcUtils::SampleCycles;

    ADC_TypeDef * instance;
    Channel channel;
    uint8_t rank;

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    AdcChannelOnChip(ADC_TypeDef * _instance, const Channel _channel, const uint8_t _rank):
            instance(_instance), channel(_channel), rank(_rank){};
    
    virtual void setSampleCycles(const SampleCycles cycles) = 0;
};


#endif



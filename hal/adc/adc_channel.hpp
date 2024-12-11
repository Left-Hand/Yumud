#pragma once

#include "sys/core/platform.h"
#include "adc_utils.hpp"

#include "analog_channel.hpp"



namespace ymd{


class AdcOnChip;
class AdcPrimary;
class AdcCompanion;

class AdcChannelConcept:public AnalogInChannel{
public:
    virtual real_t uni() = 0;
    virtual uint16_t data() = 0;

    operator real_t() override{
        return real_t(3.3f) * uni();
    }

    operator uint16_t(){
        return data();
    }
};

class AdcChannelOnChip: public AdcChannelConcept{
protected:
    using ChannelIndex = AdcUtils::ChannelIndex;
    using SampleCycles = AdcUtils::SampleCycles;

    ADC_TypeDef * instance;
    ChannelIndex channel;
    uint8_t rank;

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    AdcChannelOnChip(ADC_TypeDef * _instance, const ChannelIndex _channel, const uint8_t _rank):
            instance(_instance), channel(_channel), rank(_rank){};
    
    void init(){
        AdcUtils::installPin(channel, true);
    }

    virtual void setSampleCycles(const SampleCycles cycles) = 0;
};

}




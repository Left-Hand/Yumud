#pragma once

#include "adc_channel.hpp"

namespace ymd{

class RegularChannel: public AdcChannelOnChip{
public:
    using SampleCycles = AdcUtils::SampleCycles;
    using Channel = AdcUtils::Channel;
protected:
    uint32_t data_cache;

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;

public:
    RegularChannel(ADC_TypeDef * _instance,const Channel _channel, const uint8_t _rank):
        AdcChannelOnChip(_instance, _channel, _rank){;}


    real_t uni() override{
        return u16_to_uni(data_cache);
    }

    void setSampleCycles(const SampleCycles cycles) override{
        ADC_RegularChannelConfig(instance, (uint8_t)channel, rank, (uint8_t)cycles);
    }

};


};
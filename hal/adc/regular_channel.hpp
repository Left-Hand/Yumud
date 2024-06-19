#pragma once

#include "adc_channel.hpp"

class RegularChannel: public AdcChannelOnChip{
public:
    using SampleCycles = AdcUtils::SampleCycles;
    using Channel = AdcUtils::Channel;
protected:
    uint32_t data_cache;

    RegularChannel(ADC_TypeDef * _instance,const Channel _channel, const uint8_t _rank):
        AdcChannelOnChip(_instance, _channel, _rank){;}

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;

public:
    void setSampleCycles(const SampleCycles cycles) override{
        ADC_RegularChannelConfig(instance, (uint8_t)channel, rank, (uint8_t)cycles);
    }

    real_t uni() const override{
        real_t result;
        u16_to_uni(data_cache, result);
        return result;
    }
};

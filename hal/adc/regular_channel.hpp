#pragma once

#include "adc_channel.hpp"

namespace ymd::hal{

class AdcRegularChannel: public AdcChannelOnChip{
public:
protected:
    uint32_t data_cache;

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;

public:
    AdcRegularChannel(ADC_TypeDef * _instance,const ChannelIndex _channel, const uint8_t _rank):
        AdcChannelOnChip(_instance, _channel, _rank){;}


    real_t uni() override{
        return u16_to_uni(data_cache);
    }

    void setSampleCycles(const SampleCycles cycles) override{
        ADC_RegularChannelConfig(instance, (uint8_t)channel, rank, (uint8_t)cycles);
    }

};


};
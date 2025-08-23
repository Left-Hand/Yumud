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
    AdcRegularChannel(ADC_TypeDef * _instance,const ChannelNth nth, const uint8_t _rank):
        AdcChannelOnChip(_instance, nth, _rank){;}


    real_t uni() override{
        return u16_to_uni(data_cache);
    }

    void set_sample_cycles(const SampleCycles cycles) override{
        ADC_RegularChannelConfig(inst_, std::bit_cast<uint8_t>(nth_), rank, (uint8_t)cycles);
    }

};


};
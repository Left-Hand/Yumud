#pragma once

#include "adc_channel.hpp"

class InjectedChannel: public AdcChannelOnChip{
protected:
    uint8_t mask;

    void setCaliData(const uint16_t _cali_data){
        ADC_SetInjectedOffset(instance, mask, _cali_data);
    }

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    InjectedChannel(ADC_TypeDef * _instance, const Channel _channel, const uint8_t _rank):
            AdcChannelOnChip(_instance, _channel, _rank),
            mask((ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (rank - 1) + ADC_InjectedChannel_1){;}

    void setSampleCycles(const SampleCycles cycles) override{
        ADC_InjectedChannelConfig(instance, mask, rank, (uint8_t)cycles);
    }

    real_t uni() override{
        real_t result;
        u16_to_uni(ADC_GetInjectedConversionValue(instance, mask), result);
        return result;
    }
};
#pragma once

#include "adc_channel.hpp"

class InjectedChannel: public AdcChannelOnChip{
protected:
    using SampleCycles = AdcSampleCycles;
    using Channel = AdcChannel;

    uint8_t rank;
    uint8_t mask;
    InjectedChannel(ADC_TypeDef * _instance, const Channel _channel, const uint8_t _rank):
            AdcChannelOnChip(_instance, _channel), rank(_rank), 
            mask((ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (rank - 1) + ADC_InjectedChannel_1){;}

    void setCaliData(const uint16_t _cali_data){
        ADC_SetInjectedOffset(instance, mask, _cali_data);
    }

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    void setSampleTime(const SampleCycles _sample_time){
        ADC_InjectedChannelConfig(instance, mask, rank, (uint8_t)_sample_time);
    }

    real_t uni() const override{
        real_t result;
        u16_to_uni(ADC_GetInjectedConversionValue(instance, mask), result);
        return result;
    }
};
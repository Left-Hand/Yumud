#pragma once

#include "adc_channel.hpp"

namespace yumud{

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
    InjectedChannel(ADC_TypeDef * _instance, const Channel _channel, const uint8_t _rank);

    InjectedChannel(const InjectedChannel & other) = delete;
    InjectedChannel(InjectedChannel && other) = delete;

    void setSampleCycles(const SampleCycles cycles) override;
    

    uint16_t data() override;
    real_t uni() override;
};

};
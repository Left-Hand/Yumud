#pragma once

#include "adc_channel.hpp"

namespace ymd::hal{

class AdcInjectedChannel: public AdcChannelOnChip{
protected:
    uint8_t mask;

    void setCaliData(const uint16_t _cali_data){
        ADC_SetInjectedOffset(instance, mask, _cali_data);
    }

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    AdcInjectedChannel(ADC_TypeDef * _instance, const AdcChannelNth _channel, const uint8_t _rank);

    AdcInjectedChannel(const AdcInjectedChannel & other) = delete;
    AdcInjectedChannel(AdcInjectedChannel && other) = delete;

    void set_sample_cycles(const AdcSampleCycles cycles);
    

    uint16_t data();
    real_t uni();
};

};
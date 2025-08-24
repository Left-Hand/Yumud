#pragma once

#include "adc_channel.hpp"

namespace ymd::hal{

class AdcInjectedChannel: public AdcChannelOnChip{
protected:
    uint8_t mask;

    void set_cali_data(const uint16_t _cali_data){
        ADC_SetInjectedOffset(inst_, mask, _cali_data);
    }

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    AdcInjectedChannel(ADC_TypeDef * _instance, const AdcChannelNth nth, const uint8_t _rank);

    AdcInjectedChannel(const AdcInjectedChannel & other) = delete;
    AdcInjectedChannel(AdcInjectedChannel && other) = delete;

    void set_sample_cycles(const AdcSampleCycles cycles);
    
    uint16_t get_raw();
};

};
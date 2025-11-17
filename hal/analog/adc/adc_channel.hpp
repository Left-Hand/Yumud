#pragma once


#include "core/platform.hpp"
#include "core/sdk.hpp"

#include "adc_utils.hpp"

#include "primitive/analog_channel.hpp"



namespace ymd::hal{

class AdcOnChip;
class AdcPrimary;
class AdcCompanion;

class AdcChannelOnChip: public AnalogInIntf{
protected:
    using ChannelSelection = AdcChannelSelection;
    using SampleCycles = AdcSampleCycles;

    ADC_TypeDef * inst_;
    ChannelSelection nth_;
    uint8_t rank;

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    AdcChannelOnChip(ADC_TypeDef * _inst, const ChannelSelection nth, const uint8_t _rank):
            inst_(_inst), nth_(nth), rank(_rank){};
    
    void init(){
        adc_details::install_pin(nth_, EN);
    }

    virtual void set_sample_cycles(const SampleCycles cycles) = 0;
    virtual uint16_t get_raw() = 0;

    real_t get_voltage(){
        return (real_t(3.3f) * get_raw()) >> 12 ;
    }
};

}




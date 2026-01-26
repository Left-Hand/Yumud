#pragma once


#include "core/platform.hpp"


#include "adc_utils.hpp"





namespace ymd::hal{

class AdcOnChip;
class AdcPrimary;
class AdcCompanion;

class AdcChannelOnChip{

public:
    explicit AdcChannelOnChip(
        void * inst, 
        const AdcChannelSelection sel, 
        const uint8_t rank
    ):
            inst_(inst), sel_(sel), rank_(rank){};
    
    void init(){
        adc::details::install_pin(sel_);
    }
protected:
    using ChannelSelection = AdcChannelSelection;
    using SampleCycles = AdcSampleCycles;

    void * inst_;
    ChannelSelection sel_;
    uint8_t rank_;
    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
};

}




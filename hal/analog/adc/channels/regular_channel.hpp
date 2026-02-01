#pragma once

#include "adc_channel.hpp"

namespace ymd::hal{

class AdcRegularChannel final : public AdcChannelOnChip{
    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
    explicit AdcRegularChannel(
        void * inst,
        const ChannelSelection sel, 
        const uint8_t rank
    ):
        AdcChannelOnChip(inst, sel, rank){;}

    [[nodiscard]] uint16_t read_u16();
    void set_sample_cycles(const SampleCycles cycles);

};


};
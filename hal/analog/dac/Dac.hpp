#pragma once

#include "dac_utils.hpp"
#include "core/math/real.hpp"

#ifdef ENABLE_DAC

namespace ymd::hal{


class DacChannel{
public:
    using ChannelSelection = DacUtils::ChannelSelection;
    using Alignment = DacUtils::Alignment;
protected:
    DAC_TypeDef * instance_;
    const ChannelSelection sel_;
    const uint32_t channel_mask_;
    Alignment align_ = Alignment::R12;

    DacChannel(DAC_TypeDef * instance, ChannelSelection idx):
        instance_(instance),
        idx_(idx),
        channel_mask_(get_channel_mask(idx)){
        
        }
    DacChannel(const DacChannel & other) = delete;
    DacChannel(DacChannel && other) = delete;

    void settle();

    static uint32_t get_channel_mask(const ChannelSelection sel);
    // static uint32_t get_channel_mask(const ChannelSelection sel_);

public:
    void init();

    DacChannel & operator = (const real_t value);
};


class Dac{
protected:
    void enable_rcc(const Enable en);
public:
    void init();

    // Dac & operator = ()
};

}

#endif
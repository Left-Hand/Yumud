#pragma once

#include "DacUtils.hpp"
#include "core/math/real.hpp"

#ifdef ENABLE_DAC

namespace ymd::hal{

class DacChannel{
public:
    using ChannelIndex = DacUtils::ChannelIndex;
    using Alignment = DacUtils::Alignment;
protected:
    DAC_TypeDef * instance_;
    const ChannelIndex idx_;
    const uint32_t channel_mask_;
    Alignment align_ = Alignment::R12;

    DacChannel(DAC_TypeDef * instance, ChannelIndex idx):
        instance_(instance),
        idx_(idx),
        channel_mask_(get_channel_mask(idx)){
        
        }
    DacChannel(const DacChannel & other) = delete;
    DacChannel(DacChannel && other) = delete;

    void settle();

    static uint32_t get_channel_mask(const ChannelIndex idx);
    // static uint32_t get_channel_mask(const ChannelIndex idx);

public:
    void init();

    DacChannel & operator = (const real_t value);
};


class Dac{
protected:
    void enableRcc(const Enable en);
public:
    void init();

    // Dac & operator = ()
};

}

#endif
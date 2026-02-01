#pragma once

#include "adc_channel.hpp"

namespace ymd::hal{

class AdcInjectedChannel final:  public AdcChannelOnChip{
public:
    explicit AdcInjectedChannel(
        void * inst, 
        const AdcChannelSelection sel, 
        const uint8_t rank
    );

    AdcInjectedChannel(const AdcInjectedChannel & other) = delete;
    AdcInjectedChannel(AdcInjectedChannel && other) = delete;

    void set_sample_cycles(const AdcSampleCycles cycles);
    
    [[nodiscard]] uint16_t read_u12();

    [[nodiscard]] uq16 get_perunit() {
        //assume right aligned 12 bit resolution
        return uq16::from_bits(static_cast<uint32_t>(read_u12()) << 4);
    }

private:
    uint8_t mask_;

    void set_cali_data(const uint16_t bits);

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
};

};
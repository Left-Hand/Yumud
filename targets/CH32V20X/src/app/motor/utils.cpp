#include "utils.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "rpc.hpp"


using namespace ymd;
using namespace ymd::foc;
using namespace ymd::hal;

static __fast_inline void __ab_to_dq(DqValue & dq, const AbValue & ab, const q16 rad){
    auto [s,c] = sincos(rad);
    dq = {s * ab[1] + c * ab[0], c * ab[1] - s * ab[0]};
};

static __fast_inline void __dq_to_ab(AbValue & ab, const DqValue & dq, const q16 rad){
    auto [s,c] = sincos(rad);
    ab = {c * dq[0] - s * dq[1], c * dq[1] + s * dq[0]};
};

static __fast_inline void __ab_to_dq(DqValue & dq, const AbValue & ab, const q16 s, const q16 c){
    dq = {s * ab[1] + c * ab[0], c * ab[1] - s * ab[0]};
};

static __fast_inline void __dq_to_ab(AbValue & ab, const DqValue & dq, const q16 s, const q16 c){
    ab = {c * dq[0] - s * dq[1], c * dq[1] + s * dq[0]};
};

namespace ymd::foc{

DqCurrent ab_to_dq(const AbCurrent & ab, const q16 rad){
    DqCurrent dq;
    __ab_to_dq(dq, ab, rad);
    return dq;
}


DqVoltage ab_to_dq(const AbVoltage & ab, const q16 rad){
    DqVoltage dq;
    __ab_to_dq(dq, ab, rad);
    return dq;
}

AbCurrent dq_to_ab(const DqCurrent & dq, const q16 rad){
    AbCurrent ab;
    __dq_to_ab(ab, dq, rad);
    return ab;
}

AbVoltage dq_to_ab(const DqVoltage & dq, const q16 rad){
    AbVoltage ab;
    __dq_to_ab(ab, dq, rad);
    return ab;
}

DqCurrent ab_to_dq(const AbCurrent & ab, const q16 s, const q16 c){
    DqCurrent dq;
    __ab_to_dq(dq, ab, s, c);
    return dq;
}


DqVoltage ab_to_dq(const AbVoltage & ab, const q16 s, const q16 c){
    DqVoltage dq;
    __ab_to_dq(dq, ab, s, c);
    return dq;
}

AbCurrent dq_to_ab(const DqCurrent & dq, const q16 s, const q16 c){
    AbCurrent ab;
    __dq_to_ab(ab, dq, s, c);
    return ab;
}

AbVoltage dq_to_ab(const DqVoltage & dq, const q16 s, const q16 c){
    AbVoltage ab;
    __dq_to_ab(ab, dq, s, c);
    return ab;
}

void init_adc(){


    adc1.init(
        {
            {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
        },{
            // {AdcChannelIndex::CH5, AdcSampleCycles::T28_5},
            // {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
            // {AdcChannelIndex::CH1, AdcSampleCycles::T28_5},

            {AdcChannelIndex::CH5, AdcSampleCycles::T7_5},
            {AdcChannelIndex::CH4, AdcSampleCycles::T7_5},
            {AdcChannelIndex::CH1, AdcSampleCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T41_5},
        }
    );

    // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    adc1.set_injected_trigger(AdcInjectedTrigger::T1CC4);
    // adc1.enableContinous();
    adc1.enable_auto_inject(false);
}

}


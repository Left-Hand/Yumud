#include "utils.hpp"
#include "hal/adc/adcs/adc1.hpp"

using namespace ymd::foc;

static __fast_inline void __ab_to_dq(DqValue & dq, const AbValue & ab, const real_t rad){
    const auto s = sin(rad);
    const auto c = cos(rad);
    dq = {s * ab[1] + c * ab[0], c * ab[1] - s * ab[0]};
};

static __fast_inline void __dq_to_ab(AbValue & ab, const DqValue & dq, const real_t rad){
    const auto s = sin(rad);
    const auto c = cos(rad);
    ab = {c * dq[0] - s * dq[1], c * dq[1] + s * dq[0]};
};

namespace ymd::foc{
DqCurrent ab_to_dq(const AbCurrent & ab, const real_t rad){
    DqCurrent dq;
    __ab_to_dq(dq, ab, rad);
    return dq;
}
DqVoltage ab_to_dq(const AbVoltage & ab, const real_t rad){
    DqVoltage dq;
    __ab_to_dq(dq, ab, rad);
    return dq;
}

AbCurrent dq_to_ab(const DqCurrent & dq, const real_t rad){
    AbCurrent ab;
    __dq_to_ab(ab, dq, rad);
    return ab;
}

AbVoltage dq_to_ab(const DqVoltage & dq, const real_t rad){
    AbVoltage ab;
    __dq_to_ab(ab, dq, rad);
    return ab;
}

void init_adc(){
    using AdcChannelIndex = AdcUtils::ChannelIndex;
    using AdcCycles = AdcUtils::SampleCycles;


    adc1.init(
        {
            {AdcChannelIndex::VREF, AdcCycles::T28_5}
        },{
            {AdcChannelIndex::CH1, AdcCycles::T28_5},
            {AdcChannelIndex::CH4, AdcCycles::T28_5},
            {AdcChannelIndex::CH5, AdcCycles::T28_5},

            // {AdcChannelIndex::CH1, AdcCycles::T7_5},
            // {AdcChannelIndex::CH4, AdcCycles::T7_5},
            // {AdcChannelIndex::CH5, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T41_5},
        }
    );

    // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1CC4);
    // adc1.enableContinous();
    adc1.enableAutoInject(false);
}
}
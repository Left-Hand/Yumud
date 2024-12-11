#include "injected_channel.hpp"

using namespace ymd;

InjectedChannel::InjectedChannel(ADC_TypeDef * _instance, const ChannelIndex _channel, const uint8_t _rank):
        AdcChannelOnChip(_instance, _channel, _rank),
        mask((ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (rank - 1) + ADC_InjectedChannel_1){;}

void InjectedChannel::setSampleCycles(const SampleCycles cycles){
    ADC_InjectedChannelConfig(instance, mask, rank, (uint8_t)cycles);
}

uint16_t InjectedChannel::data(){
    return ADC_GetInjectedConversionValue(instance, mask);
}

real_t InjectedChannel::uni(){
    return u16_to_uni(ADC_GetInjectedConversionValue(instance, mask) << 4);
}
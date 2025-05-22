#include "injected_channel.hpp"

using namespace ymd::hal;

AdcInjectedChannel::AdcInjectedChannel(ADC_TypeDef * _instance, const AdcChannelIndex _channel, const uint8_t _rank):
        AdcChannelOnChip(_instance, _channel, _rank),
        mask((ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (rank - 1) + ADC_InjectedChannel_1){;}

void AdcInjectedChannel::set_sample_cycles(const AdcSampleCycles cycles){
    ADC_InjectedChannelConfig(instance, mask, rank, (uint8_t)cycles);
}

uint16_t AdcInjectedChannel::data(){
    return ADC_GetInjectedConversionValue(instance, mask);
}

real_t AdcInjectedChannel::uni(){
    return iq_t<18>(ADC_GetInjectedConversionValue(instance, mask)) >> 12;
}
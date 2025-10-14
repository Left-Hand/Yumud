#include "injected_channel.hpp"

using namespace ymd::hal;

AdcInjectedChannel::AdcInjectedChannel(ADC_TypeDef * _instance, const AdcChannelSelection _channel, const uint8_t _rank):
        AdcChannelOnChip(_instance, _channel, _rank),
        mask((ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (rank - 1) + ADC_InjectedChannel_1){;}

void AdcInjectedChannel::set_sample_cycles(const AdcSampleCycles cycles){
    ADC_InjectedChannelConfig(inst_, mask, rank, (uint8_t)cycles);
}

uint16_t AdcInjectedChannel::get_raw(){
    return ADC_GetInjectedConversionValue(inst_, mask);
}
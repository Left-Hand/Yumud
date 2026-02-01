#include "injected_channel.hpp"
#include "core/sdk.hpp"

using namespace ymd::hal;


#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, ADC_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


AdcInjectedChannel::AdcInjectedChannel(
    void * inst, 
    const AdcChannelSelection sel, 
    const uint8_t rank
):
    AdcChannelOnChip(inst, sel, rank),
    mask_((ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (rank_ - 1) + ADC_InjectedChannel_1){;}

void AdcInjectedChannel::set_sample_cycles(const AdcSampleCycles cycles){
    ADC_InjectedChannelConfig(SDK_INST(inst_), mask_, rank_, static_cast<uint8_t>(cycles));
}

uint16_t AdcInjectedChannel::read_u12() {
    return ADC_GetInjectedConversionValue(SDK_INST(inst_), mask_);
}

void AdcInjectedChannel::set_cali_data(const uint16_t bits){
    ADC_SetInjectedOffset(SDK_INST(inst_), mask_, bits);
}

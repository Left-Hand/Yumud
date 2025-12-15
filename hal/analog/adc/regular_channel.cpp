#include "regular_channel.hpp"
#include "core/sdk.hpp"

using namespace ymd::hal;


#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, ADC_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


void AdcRegularChannel::set_sample_cycles(const SampleCycles cycles) {
    ADC_RegularChannelConfig(
        SDK_INST(inst_), std::bit_cast<uint8_t>(sel_), rank_, static_cast<uint8_t>(cycles)
    );
}
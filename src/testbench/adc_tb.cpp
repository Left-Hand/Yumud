#include "tb.h"

void adc_tb(OutputStream & logger){

    adc1.init(
        {},{
            AdcChannelConfig{.channel = AdcChannels::CH3, .sample_cycles = AdcSampleCycles::T71_5},
            AdcChannelConfig{.channel = AdcChannels::CH4, .sample_cycles = AdcSampleCycles::T71_5}
        });
    adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    timer3.oc(4).cnt() = 0;
    adc1.setPga(AdcOnChip::Pga::X64);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    TIM3->CH4CVR = TIM3->ATRLR >> 1;
    adc1.enableCont();
    adc1.enableScan();
    adc1.enableAutoInject();

    auto & bled = portC[13];
    bled.OutPP();
}
#include "tb.h"


// PA^8 -> LED 
// PA^9 -> DEBUGGER^URX

// format "%f,%f,%f"
// DATA[0] <- volt(PA^0)
// DATA[1] <- volt(PA^1)
// DATA[4] <- temprature()

void adc_tb(OutputStream & logger){

    adc1.init(
        {},{
            AdcChannelConfig{.channel = AdcChannels::TEMP, .sample_cycles = AdcSampleCycles::T71_5},
            AdcChannelConfig{.channel = AdcChannels::CH1, .sample_cycles = AdcSampleCycles::T71_5},
            AdcChannelConfig{.channel = AdcChannels::TEMP, .sample_cycles = AdcSampleCycles::T239_5},
            AdcChannelConfig{.channel = AdcChannels::VREF, .sample_cycles = AdcSampleCycles::T239_5},
        });

    adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    adc1.setPga(AdcOnChip::Pga::X64);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);

    timer3.oc(4).cnt() = timer3.arr() >> 1;

    adc1.enableCont();
    adc1.enableScan();
    adc1.enableAutoInject();

    auto & bled = portA[8];
    bled.OutPP();

    while(true){
        static int blink_cnt = 0;
        blink_cnt = (blink_cnt+1) % 100;
        if(blink_cnt == 0){bled = !bled;}
        adc1.swStartInjected();

        auto Vsense = real_t(uint16_t(ADC1->IDATAR3)) * (3.3/4096);           // 电压值 
        auto temperate = ((1.43 - Vsense) / 0.0043 + 25);    // 转换为温度值，转换公式：T（℃）= ((V25 - Vsense) / Avg_Slope) + 25

        logger.println(ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3, ADC1->IDATAR4, temperate);
        delay(2);
    }
}
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
            // AdcChannelConfig{.channel = AdcChannel::TEMP, .sample_cycles = AdcSampleCycles::T239_5},
            // AdcChannelConfig{.channel = AdcChannel::VREF, .sample_cycles = AdcSampleCycles::T239_5},
            // AdcChannelConfig{.channel = AdcChannel::CH1, .sample_cycles = AdcSampleCycles::T239_5},
            AdcChannelConfig{.channel = AdcChannel::CH0, .sample_cycles = AdcSampleCycles::T239_5},
        });

    adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T3CC4);

    // adc1.enableContinous();
    adc1.enableAutoInject();

    // auto & bled = portA[8];
    // bled.OutPP();
    timer1.init(36000);
    auto & pwm = timer1.oc(1);
    pwm.init(true);

    real_t src_data;
    real_t raw_data;
    real_t filt_data;
    LowpassFilter_t<real_t, real_t> lpf(200);

    timer3.init(40000);
    timer3.oc(4).init(true);
    timer3.oc(4) = 0.5;
    timer3.bindCb(TimerUtils::TimerIT::CC4, [&](){
        real_t angle = TAU * t * 50;
        src_data = 0.5 + 0.2 * sin(angle) + 0.2 * sin(angle * 64);
        pwm = src_data;
        raw_data = int(ADC1->IDATAR1);
        filt_data = lpf.update(raw_data, t);
        Sys::Clock::reCalculateTime();
    });

    timer3.enableIt(TimerUtils::TimerIT::CC4, {0,0});

    while(true){
        // bled = (millis() / 2000) & 0b1;
        // adc1.swStartInjected();

        // auto Vsense = (real_t(uint16_t(ADC1->IDATAR1)) >> 12) * 3.3;           // 电压值 
        // auto temperate = ((1.43 - Vsense) / 0.0043 + 25);    // 转换为温度值，转换公式：T（℃）= ((V25 - Vsense) / Avg_Slope) + 25
        // auto temperate = TempSensor_Volt_To_Temper(ADC1->IDATAR1 * 3300 / 4096);
        // auto vref = 3.3 * (real_t(uint16_t(ADC1->IDATAR2)) >> 12);
        // logger.println(ADC1->IDATAR3, ADC1->IDATAR4, temperate, vref, ADC1->IDATAR2);
        logger.println(int(src_data << 12), int(raw_data), int(filt_data));


        // delay(1);
    }
}
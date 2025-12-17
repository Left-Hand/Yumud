#include "src/testbench/tb.h"
#include "core/stream/ostream.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/dma/dma.hpp"
#include "core/sdk.hpp"

// #define ADC_TB_MAIN
// #define ADC_TB_REGULAR_BLOCKING
#define ADC_TB_REGULAR_DMA
// #define ADC_TB_INJECT


// PA^8 -> LED 
// PA^9 -> DEBUGGER^URX

// format "%f,%f,%f"
// DATA[0] <- volt(PA^0)
// DATA[1] <- volt(PA^1)
// DATA[4] <- temprature()

using namespace ymd;


void adc_tb(OutputStream & logger){
    #ifdef ADC_TB_MAIN

    hal::adc1.init(
        {
            hal::AdcChannelConfig{hal::AdcChannelSelection::CH0},
        },{
            hal::AdcChannelConfig{hal::AdcChannelSelection::TEMP, hal::AdcCycleEnum::T239_5},
            hal::AdcChannelConfig{hal::AdcChannelSelection::VREF, hal::AdcCycleEnum::T239_5},
            hal::AdcChannelConfig{hal::AdcChannelSelection::CH0, hal::AdcCycleEnum::T239_5},
            hal::AdcChannelConfig{hal::AdcChannelSelection::CH1, hal::AdcCycleEnum::T239_5},
        });

    hal::adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T3CC4);
    hal::adc1.enableContinous();
    hal::adc1.enableAutoInject();

    timer1.init(36000);
    auto & pwm = timer1.oc(1);
    pwm.init();

    real_t src_data;
    real_t raw_data;
    real_t filt_data;
    LowpassFilter<real_t, real_t> lpf(200);

    timer3.init(40000);
    timer3.oc(4).init();
    timer3.oc(4) = 0.5;
    timer3.bindCb(TimerIT::CC4, [&](){
        real_t angle = TAU * t * 50;
        src_data = 0.5 + 0.2 * sin(angle) + 0.2 * sin(angle * 64);
        pwm = src_data;
        raw_data = int(ADC1->IDATAR1);
        filt_data = lpf.update(raw_data, t);
        Sys::Clock::reCalculateTime();
    });

    timer3.enableIt(TimerIT::CC4, {0,0});

    while(true){
        // bled = (millis() / 2000) & 0b1;
        // hal::adc1.swStartInjected();

        // auto Vsense = (real_t(uint16_t(ADC1->IDATAR1)) >> 12) * 3.3;           // ��ѹֵ 
        // auto temperate = ((1.43 - Vsense) / 0.0043 + 25);    // ת��Ϊ�¶�ֵ��ת����ʽ��T���棩= ((V25 - Vsense) / Avg_Slope) + 25
        auto temperate = TempSensor_Volt_To_Temper(ADC1->IDATAR1 * 3300 / 4096);
        auto vref = ADC1->IDATAR2 * 3300 / 4096;
        logger.println(ADC1->IDATAR3, ADC1->IDATAR4, temperate, vref);
        // logger.println(int(src_data << 12), int(raw_data), int(filt_data));


        // clock::delay(1ms);
    }
    #endif

    #ifdef ADC_TB_REGULAR_BLOCKING
    hal::adc1.init(
        {
            hal::AdcChannelConfig{hal::AdcChannelSelection::TEMP, hal::AdcCycleEnum::T239_5},
            hal::AdcChannelConfig{hal::AdcChannelSelection::VREF, hal::AdcCycleEnum::T239_5},
        },{
            // hal::AdcChannelConfig{AdcChannelSelection::CH0},
        });

    while(true){
        hal::adc1.swStartRegular(true);

        while(true){
            clock::delay(1ms);
            bool still_waiting = !hal::adc1.isIdle();
            while(!hal::adc1.isIdle()){
                logger.println(".");
                clock::delay(200ms);
            }
            if(still_waiting) logger.println();
            break;
        };
        logger.println(hal::adc1.getConvResult() * 3300 / 4096);
    }
    #endif

    #ifdef ADC_TB_REGULAR_DMA
    
    // RegularChannel rch{ADC1, AdcChannel::CH0, 0};

    hal::adc1.init(
        {
            hal::AdcChannelConfig{hal::AdcChannelSelection::TEMP, hal::AdcSampleCycles::T239_5},
            hal::AdcChannelConfig{hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T239_5},
            // hal::AdcChannelConfig{hal::AdcChannelSelection::CH0, hal::AdcCycleEnum::T239_5},
            // hal::AdcChannelConfig{hal::AdcChannelSelection::CH1, hal::AdcCycleEnum::T239_5},
        },{
            // hal::AdcChannelConfig{hal::AdcChannelSelection::CH0},
        }, {});

    std::array<uint16_t, 16> adc_dma_buf;
    hal::dma1_ch1.init({hal::DmaMode::ToMemCircular, hal::DmaPriority::High});
    hal::dma1_ch1.start_transfer_pph2mem<uint16_t>(adc_dma_buf.data(), &(ADC1->RDATAR), adc_dma_buf.size());

    hal::adc1.enable_dma(EN);
    hal::adc1.sw_start_regular(true);
    while(true){
        logger.println(adc_dma_buf[0], hal::dma1_ch1.pending_count());
    }

    #endif

    #ifdef ADC_TB_INJECT
    InjectedChannel jch{ADC1, AdcChannel::CH0, 0};
    #endif

}
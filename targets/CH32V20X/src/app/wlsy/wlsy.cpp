#include "wlsy_inc.hpp"
#include "heat.hpp"
#include "tjc.hpp"
#include "input.hpp"
#include "scenes.hpp"
#include "frontend.hpp"

namespace WLSY{

using LowpassFilter = LowpassFilter_t<real_t, real_t>;
using Sys::t;
using Sys::Clock::reCalculateTime;
using Waveform = TJC::Waveform;
using WaveWindow = TJC::WaveWindow;

}



void wlsy_main(){
    using namespace WLSY;

    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);
    TJCER.init(921600, CommMethod::Blocking);
    auto & led_gpio = portA[7];
    led_gpio.outpp(1);


    auto &              trig_gpio(portA[0]);
    trig_gpio.inpu();
    ExtiChannel         trig_ecti_ch(trig_gpio, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti  cap(trig_ecti_ch, trig_gpio);
    SpeedCapture speedCapture{cap};

    speedCapture.init();

    auto & scl_gpio = portB[15];
    auto & sda_gpio = portB[14];

    I2cSw               i2csw{scl_gpio, sda_gpio};
    i2csw.init(100000);


    INA226 ina226{i2csw};
    // ina226.init(real_t(0.009), real_t(5));
    // ina226.update();

    timer1.init(120'000);
    timer1.initBdtr(100);


    timer1.oc(4).init(TimerUtils::OcMode::UpValid, false);

    TIM_OCInitTypeDef  TIM_OCInitStructure;

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    timer1.oc(4).cvr() = timer1.arr()-1;

    auto & ch = timer1.oc(1);
    auto & chn = timer1.ocn(1);
    auto  & en_gpio = portB[0];

    using AdcChannelEnum = AdcUtils::Channel;
    using AdcCycleEnum = AdcUtils::SampleCycles;

    adc1.init(
        {
            AdcChannelConfig{AdcChannelEnum::VREF, AdcCycleEnum::T28_5}
        },{
            AdcChannelConfig{AdcChannelEnum::CH4, AdcCycleEnum::T28_5},
            AdcChannelConfig{AdcChannelEnum::CH5, AdcCycleEnum::T28_5},
        }
    );

    // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1TRGO);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1CC4);
    // adc1.enableContinous();
    adc1.enableAutoInject(false);

    HX711 hx711(portA[6], portA[1]);
    hx711.init();
    hx711.compensate();



    en_gpio.outpp(0);
    ch.setIdleState(true);
    ch.init();
    chn.setIdleState(false);
    chn.init();


    NTC ntc_h{0};
    NTC ntc_l{1};

    Buck buck{ina226, ch};
    Heater heater{en_gpio, ntc_h, buck};


    InputModule inputMachine{ina226, heater, ntc_l, ntc_h};

    OutputModule outputMachine{speedCapture, hx711};

    BackModule machine{inputMachine, outputMachine};

    TJC tjc{uart1};
    tjc.init();

    TJC::WaveWindow input_wavewindow{tjc, 1};
    TJC::WaveWindow output_wavewindow{tjc, 2};

    PowerInScene input_scene = PowerInScene{
        machine,{
        new TJC::Label {tjc, "input", "current"},
        new TJC::Label {tjc, "input", "voltage"},
        new TJC::Label {tjc, "input", "power"},
        new TJC::Label {tjc, "input", "TL"},
        new TJC::Label {tjc, "input", "TH"},

        new TJC::Waveform {input_wavewindow, 0, {0, 4}},
        new TJC::Waveform {input_wavewindow, 1, {0, 100}},
        new TJC::Waveform {input_wavewindow, 2, {0, 100}}
        }
    };


    PowerOutScene output_scene{
        machine,{
        new TJC::Label {tjc, "output", "speed"},
        new TJC::Label {tjc, "output", "force"},
        new TJC::Label {tjc, "output", "power"},
        new TJC::Waveform {output_wavewindow, 0, {0, 16}},
        new TJC::Waveform {output_wavewindow, 1, {0, real_t(0.4)}},
        new TJC::Waveform {output_wavewindow, 2, {0, 1}}
        }
    };


    ExamScene exam_scene{
        machine,{
        new TJC::Label {tjc, "result", "spower"},
        new TJC::Label {tjc, "result", "inputpower"},
        new TJC::Label {tjc, "result", "outpower"},
        new TJC::Label {tjc, "result", "efficiency"},
        }
    };

    MainScene main_scene{machine, {&input_scene, &output_scene, &exam_scene}};

    FrontModule interact{machine, main_scene};


    timer2.init(4000);
    timer2.bindCb(TimerUtils::IT::Update, [&](){
        ntc_l.update();
        ntc_h.update();
        buck.run();
    });
    timer2.enableIt(TimerUtils::IT::Update, {1,1});

    machine.init();
    interact.init();

    while(true){
        // DEBUG_PRINTLN(speedCapture.getSpeed(), speedCapture.rad, speedCapture.dur, hx711.getWeightGram(), hx711.getNewton());
        machine.run();
        interact.run();

        // DEBUG_PRINTLN( std::setprecision(4),
        //     hx711.getNewton(),
        //     ina226.getVoltage(), ina226.getCurrent(), ntc_h.getTemp(), ntc_l.getTemp(), speedCapture.getSpeed());
        led_gpio = (millis() / 200) % 2;
        // DEBUG_PRINTLN(hx711.getNewton());
        // DEBUG_PRINTLN(ntc_h.getTemp(), ntc_l.getTemp());
        // DEBUG_PRINTLN(ina226.getCurrent(), ina226.getVoltage(), ina226.getPower());
        // if(speedCapture.getSpeed() < 1){
        //     heater.on();
        // }else{
        //     heater.off();6
        // }
        reCalculateTime();
    }
}
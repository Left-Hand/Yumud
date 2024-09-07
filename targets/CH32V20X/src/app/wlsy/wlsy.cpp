#include "wlsy_inc.hpp"
#include "heat.hpp"
#include "tjc.hpp"
#include "input.hpp"
#include "scenes.hpp"

namespace WLSY{

using LowpassFilter = LowpassFilter_t<real_t, real_t>;
using Sys::t;
using Sys::Clock::reCalculateTime;
using Waveform = TJC::Waveform;
using WaveWindow = TJC::WaveWindow;





class FrontModule{
public:
    using ColorType = RGB565;
    // ST7789 & ds;
    BackModule & bm;

    enum class SceneIndex:uint8_t{
        MAIN, 
        GRAPH_INPUT,
        GRAPH_OUTPUT
    };
    
    SceneIndex scene_index = SceneIndex::MAIN;
    Painter<ColorType> painter;


    using Fifo = RingBuf_t<real_t, 128>;



    void drawMain(){
        auto im_info = bm.getInputModuleInfos();

        painter.drawString({0, 0},  "输入电流/A: " + String(im_info.amps));
        painter.drawString({0, 16},  "输入电压/V: " + String(im_info.volt));
        painter.drawString({0, 32}, "输入功率/W: " + String(im_info.watt));

        // auto om_info = bm.getOutputModuleInfos();

        // painter.drawString({0, 24},  "速度/M/s:  " + toString(om_info.speed));
        // painter.drawString({0, 32},  "阻力/N:    " + toString(om_info.force));
        // painter.drawString({0, 40}, "输出功率/W:" + toString(om_info.watt));

        // painter.drawString({0, 48}, "效率:      " + toString(bm.getEffiency() * 100) + "%");
        // painter.drawString({0, 48 + 8}, "臭写单片机的单干物理实验竞赛 寄");
    };

    Fifo amps_fifo;
    Fifo watt_fifo;
    Fifo volt_fifo;

    void drawGraphInput(){
        // auto im_info = bm.getInputModuleInfos();

        // amps_fifo.push_back(im_info.amps);
        // watt_fifo.push_back(im_info.watt);
        // volt_fifo.push_back(im_info.volt);

    };

    void drawGraphOutput(){

    }
public:
    FrontModule( BackModule & _bm): bm(_bm){;}

    void init(){

        // ds.enableFlipX(false);
        // ds.enableFlipY(false);

        // painter.bindImage(ds.fetchFrame());
        // painter.bindImage(ds);
        // painter.setColor(ColorType::WHITE);
        // painter.setFontScale(2);
        // painter.setChFont(&font7x7);
        // painter.setEnFont(&font8x5);
    }

    void run(){
        painter.flush(Binary::BLACK);
    
        switch(scene_index){
            case SceneIndex::MAIN:
                drawMain();
                break;
            case SceneIndex::GRAPH_INPUT:
                drawGraphInput();
                break;
            case SceneIndex::GRAPH_OUTPUT:
                drawGraphOutput();
                break;
        }
        // mill = millis
        // ds.update();
    }

};


}



void wlsy_main(){
    using namespace WLSY;

    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    auto & led_gpio = portA[7];
    led_gpio.outpp(1);

    auto &              trig_gpio(portA[0]);
    trig_gpio.inpu();
    ExtiChannel         trig_ecti_ch(trig_gpio, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti  cap(trig_ecti_ch, trig_gpio);

    auto & scl_gpio = portB[15];
    auto & sda_gpio = portB[14];

    I2cSw               i2csw{scl_gpio, sda_gpio};
    i2csw.init(100000);


    INA226 ina226{i2csw};
    ina226.init(real_t(0.009), real_t(5));
    ina226.update();

    timer1.init(240'000);
    timer1.initBdtr(20);


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



    en_gpio.outpp(0);
    ch.setIdleState(true);
    ch.init();
    chn.setIdleState(false);
    chn.init();

    en_gpio.set();
    ch = real_t(0.1);


    HX711 hx711(portA[6], portA[1]);
    hx711.init();
    hx711.compensate();

    while(true){
        ch = real_t(0.8) + sin(8 * t) * real_t(0.05);
        ina226.update();
        hx711.update();
        DEBUG_PRINTLN( std::setprecision(4),
            hx711.getNewton(),
            ina226.getVoltage(), ina226.getCurrent(), ADC1->IDATAR1, bool(trig_gpio));

        // DEBUG_PRINTLN(bool(portA[8]), bool(portB[13]), bool(en_gpio), TIM1->CH1CVR, TIM1->ATRLR);
    }





    NTC ntc_h{1};
    NTC ntc_l{0};

    Buck buck{ina226, ch};
    Heater heater{portB[0], ntc_h, buck};


    InputModule inputMachine{ina226, heater, ntc_l, ntc_h};


    SpeedCapture speedCapture{cap};

    OutputModule outputMachine{speedCapture, hx711};

    // HC12 hc12{uart1};

    // SpiDrv SpiDrvLcd = SpiDrv(spi2, 0);
    // DisplayInterfaceSpi SpiInterfaceLcd(SpiDrvLcd, portD[7], portB[7]);
    // ST7789 tftDisplayer(SpiInterfaceLcd, Vector2i(240,240));
    // {//init tft
    //     tftDisplayer.init();
    //     tftDisplayer.setFlipX(true);
    //     tftDisplayer.setFlipY(false);
    //     tftDisplayer.setSwapXY(true);
    //     tftDisplayer.setFormatRGB(true);
    //     tftDisplayer.setFlushDirH(false);
    //     tftDisplayer.setFlushDirV(false);
    //     tftDisplayer.setInversion(true);

    //     // tftDisplayer.init();
    //     // tftDisplayer.setDisplayOffset(Vector2i(1, 26));
    //     // tftDisplayer.setFlipX(true);
    //     // tftDisplayer.setFlipY(false);
    //     // tftDisplayer.setSwapXY(true);
    //     // tftDisplayer.setFormatRGB(false);
    //     // tftDisplayer.setFlushDirH(false);
    //     // tftDisplayer.setFlushDirV(false);
    //     // tftDisplayer.setInversion(true);
    // }
    // Painter<RGB565> painter = Painter<RGB565>();
    // painter.bindImage(tftDisplayer);

    // {
    //     painter.flush(RGB565::GREEN);
    //     painter.flush(RGB565::BLUE);
    // }
    BackModule machine{inputMachine, outputMachine};

    TJC tjc{uart1};
    tjc.init();

    TJC::WaveWindow input_wavewindow{tjc, 4};
    TJC::WaveWindow output_wavewindow{tjc, 4};

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

    FrontModule interact{machine};
    machine.init();
    interact.init();

    while(true){
        // DEBUG_PRINT(speedCapture.getSpeed(), speedCapture.rad, speedCapture.dur, hx711.getWeightGram(), hx711.getNewton());
        machine.run();
        interact.run();

        // if(speedCapture.getSpeed() < 1){
        //     heater.on();
        // }else{
        //     heater.off();6
        // }
        reCalculateTime();
    }
}
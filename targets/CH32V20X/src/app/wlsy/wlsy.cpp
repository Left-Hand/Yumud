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
    // DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Dma, CommMethod::Interrupt);
    auto & led_gpio = portA[7];
    led_gpio.outpp(1);


    auto & scl_gpio = portB[15];
    auto & sda_gpio = portB[14];

    I2cSw               i2csw{scl_gpio, sda_gpio};
    i2csw.init(100000);


    INA226 ina226{i2csw};
    ina226.init(real_t(0.009), real_t(5));
    ina226.update();

    timer1.init(240'000);
    timer1.initBdtr(20);

    auto & ch = timer1.oc(1);
    auto & chn = timer1.ocn(1);
    auto  & en_gpio = portB[0];

    // while(true){
    //     DEBUG_PRINTLN(t, ina226.getVoltage(), ina226.getCurrent());
    //     delay(100);
    //     led_gpio = !led_gpio;
    // }

    en_gpio.outpp(0);
    ch.setIdleState(true);
    ch.init();
    chn.setIdleState(false);
    chn.init();

    en_gpio.set();
    ch = real_t(0.1);

    while(true){
        ch = real_t(0.8) + sin(8 * t) * real_t(0.05);
        ina226.update();
        DEBUG_PRINTLN( std::setprecision(4),
            ina226.getVoltage(), ina226.getCurrent());
        // DEBUG_PRINTLN(bool(portA[8]), bool(portB[13]), bool(en_gpio), TIM1->CH1CVR, TIM1->ATRLR);
    }


    // adc1.init(
    //     {
    //         AdcChannelConfig{.channel = AdcChannels::TEMP, .sample_cycles = AdcSampleCycles::T55_5}
    //     },
    //     {
    //         AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T55_5}
    //     }
    // );

    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    // adc1.enableCont();
    // adc1.enableAutoInject();



    auto &              trigGpioA(portA[0]);
    ExtiChannel         trigExtiCHA(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti  capA(trigExtiCHA, trigGpioA);

    NTC ntc;
    Heater heater{portB[0], ntc};


    InputModule inputMachine{ina226, heater};

    HX711 hx711(portA[6], portA[1]);

    // hx711.init();
    // hx711.compensate();

    // while(true){
    //     hx711.update();
    //     DEBUG_PRINT(hx711.getWeightGram());
    // }
    SpeedCapture speedCapture{capA};

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
    FrontModule interact{machine};


    machine.init();
    interact.init();


    while(true){
        // DEBUG_PRINT(speedCapture.getSpeed(), speedCapture.rad, speedCapture.dur, hx711.getWeightGram(), hx711.getNewton());
        machine.run();
        interact.run();

        // static LowpassFilter lpf(1.0);
        // auto s = machine.getOutputModuleInfos().speed;
        // auto n = lpf.update(machine.getOutputModuleInfos().force, t);
        adc1.swStartInjected();
        DEBUG_PRINTLN(hx711.getWeightGram(),speedCapture.getSpeed());
        // hx711.update();
        // if(speedCapture.getSpeed() < 1){
        //     heater.on();
        // }else{
        //     heater.off();6
        // }
        reCalculateTime();
    }
}
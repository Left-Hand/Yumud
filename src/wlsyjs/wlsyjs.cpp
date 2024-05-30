#include "types/real.hpp"

#include "dsp/filter/LowpassFilter.hpp"

#include "src/device/Adc/HX711/HX711.h"
#include "src/gpio/port_virtual.hpp"
#include "src/bus/bus_inc.h"
#include "src/debug/debug_inc.h"
#include "src/exti/exti.hpp"
#include "src/timer/capture_channel.hpp"
#include "src/adc/adcs/adc1.hpp"

#include "src/device/Adc/INA226/ina226.hpp"
#include "src/system.hpp"

#include "src/device/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "src/device/Adc/HX711/HX711.hpp"

#include "types/image/painter.hpp"

namespace WLSY{

using LowpassFilter = LowpassFilter_t<real_t, real_t>;
using Sys::t;
using Sys::Clock::reCalculateTime;

class SpeedCapture{
// protected:
public:
    LowpassFilter lpf{10.0};
    CaptureChannelExti & cap;
    real_t dur;//ms
    
    static constexpr uint8_t poles = 9;
    static constexpr real_t rad_delta = TAU / poles;
    static constexpr real_t radius = 0.05;
public:
    real_t rad;
    real_t omega;
    real_t speed;

    SpeedCapture(CaptureChannelExti & cap):cap(cap){};

    void init(){
        dur = 0;
        rad = 0;
        omega = 0;
        speed = 0;

        cap.init();
        cap.bindCb([this](){this->update();});
    }

    void update(){
        dur = lpf.update(cap.getPeriodUs() / 1000.0, t);
        rad += rad_delta;
        omega = rad_delta / (dur / 1000);
        speed = omega * radius;
    }

    real_t getSpeed(){
        return speed;
    }
};

class TempSensor{

};

class Heater{
protected:
    Gpio & heat_gpio;
public:
    Heater(Gpio & _heat_gpio):heat_gpio(_heat_gpio){}

    void init(){heat_gpio.OutPP();}

    void on(){heat_gpio.set();}

    void off(){heat_gpio.clr();}
};

class WattMonitor{
    virtual real_t getWatt() = 0;
};

class InputModule:public WattMonitor{
protected:
    INA226 & ina;
    Heater & ht;
public:
    InputModule(INA226 & _ina, Heater & _ht):ina(_ina), ht(_ht){;}

    real_t getWatt() override{
        // real_t watt = ina.update();
        real_t watt = 36.0;
        return MAX(watt, 0);
    }

    void init(){
        ina.init(0.1, 5.0);
        ht.init();
    }

    void run(){

    }
};

class OutputModule:public WattMonitor{
// protected:
public:
    SpeedCapture & sc;
    HX711 & hx;
public:
    OutputModule(SpeedCapture & _sc, HX711 & _hx):sc(_sc), hx(_hx){;}
    real_t getWatt() override{
        real_t watt = hx.getNewton();
        return MAX(watt, 0);
    }

    void init(){
        sc.init();
        hx.init();
        hx.setConvType(HX711::ConvType::A128);
        hx.compensate();
    }

    void run(){
        hx.update();
    }
};



class BackModule{
public:
    InputModule & im;
    OutputModule & om;
public:
    BackModule(InputModule & _im, OutputModule & _om):im(_im), om(_om){;}

    void init(){
        im.init();
        om.init();
    }

    void run(){
        im.run();
        om.run();
    }

    // real_t getSpeed(){
        // return im..getSpeed();
    // }

    real_t getInputWatt(){
        return im.getWatt();
    }

    real_t getOutputWatt(){
        return om.getWatt();
    }

    real_t getEffiency(){
        return om.getWatt() / im.getWatt();
    }
};

class FrontModule{
public:
    SSD13XX & ds;
    Painter<Binary> painter;
    BackModule & bm;

public:
    FrontModule(SSD13XX & _ds, BackModule & _bm):ds(_ds), bm(_bm){;}

    void init(){
        ds.init();
        ds.enableFlipX(false);
        ds.enableFlipY(false);

        painter.bindImage(ds.fetchFrame());
        painter.setColor(true);
    }

    void run(){
        painter.flush(Binary::BLACK);

        static int cnt = 0;
        cnt++;

        // painter.setFont(font7x7);
        // painter.drawString({0, 24}, "N:");
        // painter.drawString({0, 0}, "入瓦");
        // painter.drawString({0, 8}, "出瓦" + toString(bm.getOutputWatt()));
        // painter.drawString({0, 16}, "效率" + toString(bm.getEffiency()));

        painter.setFont(font8x6);
        painter.drawString({20, 0}, toString(cnt));

        // ds.fill(false);
        // oled.update();
        ds.update();
    }

};

void app_main(){
    I2cSw               i2csw(portB[2], portB[10]);
    i2csw.init(0);

    auto &              trigGpioA(portA[0]);
    ExtiChannel         trigExtiCHA(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti  capA(trigExtiCHA, trigGpioA);


    Heater heater{portA[4]};
    I2cDrv ina226_drv(i2csw, INA226::default_id);
    INA226 ina226(ina226_drv);

    InputModule inputMachine{ina226, heater};

    HX711 hx711(portB[1], portB[0]);
    SpeedCapture speedCapture{capA};

    OutputModule outputMachine{speedCapture, hx711};



    // OledInterfaceI2c oled_drv(i2csw, SSD13XX::default_id);
    spi1.init(1000000, SerBus::TxOnly);
    spi1.bindCsPin(portA[15], 0);

    SpiDrv spi1drv{spi1, 0};
    
    DisplayInterfaceSpi oled_drv{spi1drv, portB[4], portA[9]};
    SSD13XX_128X64 oled(oled_drv);


    BackModule machine{inputMachine, outputMachine};
    FrontModule interact{oled, machine};


    machine.init();
    interact.init();


    while(true){
        // DEBUG_PRINT(speedCapture.getSpeed(), speedCapture.rad, speedCapture.dur, hx711.getWeightGram(), hx711.getNewton());
        machine.run();
        interact.run();
        // hx711.update();
        // if(speedCapture.getSpeed() < 1){
        //     heater.on();
        // }else{
        //     heater.off();
        // }
        reCalculateTime();
    }
}


}

int main(){
    Sys::Misc::prework();
    uart2.init(115200 * 4);
    delay(200);
    WLSY::app_main();
    return 0;
}
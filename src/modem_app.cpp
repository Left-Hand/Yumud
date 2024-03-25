#include "apps.h"

#include "src/device/CommonIO/Led/rgbLed.hpp"
#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "fwwb_compents/fwwb_inc.h"

using Color = Color_t<real_t>;
Printer & logger = uart2;
constexpr uint32_t mo_freq = 3800;


//Laser PB8 TM4CH3
//TrigIn PA4
void modem_app(){
    using namespace FWWB;

    uart2.init(115200*4);
    logger.setEps(4);

    can1.init(Can::BaudRate::Kbps125);
    can1.enableHwReTransmit(false);


    auto ws_out = Gpio(GPIOB, Pin::_8);
    auto leds = WS2812Chain<3>(ws_out);
    leds.init();


    auto trigGpioA = Gpio(GPIOA, Pin::_4);
    auto trigExtiCHA = ExtiChannel(trigGpioA, 1, 0, ExtiChannel::Trigger::RisingFalling);
    auto capA = CaptureChannelExti(trigExtiCHA, trigGpioA);
    auto demodemA = SimpleDeModem(capA, mo_freq);
    auto panelTargetA = PanelTarget(demodemA, 2, 0);

    // auto ledA = WS2812(Gpio(GPIOB, Pin::_8));
    auto panelLedA = PanelLed(leds[0]);


    auto trigGpioB = Gpio(GPIOA, Pin::_0);
    auto trigExtiCHB = ExtiChannel(trigGpioB, 1, 0, ExtiChannel::Trigger::RisingFalling);
    auto capB = CaptureChannelExti(trigExtiCHB, trigGpioB);
    auto demodemB = SimpleDeModem(capB, mo_freq);
    auto panelTargetB = PanelTarget(demodemB, 2, 1);

    // auto ledB = WS2812(Gpio(GPIOB, Pin::_8));
    auto panelLedB = PanelLed(leds[2]);

    auto panelUnitA = PanelUnit(panelTargetA, panelLedA);
    auto panelUnitB = PanelUnit(panelTargetB, panelLedB);

    auto can_station = CanStation(can1, logger, 0);
    auto station = TargetStation(can_station, panelUnitA, panelUnitB);
    station.init();
    // auto led0 = AW9523::RgbLed(aw, Pin::_2, Pin::_3, Pin::_4);
    // led0.init();
    // led0.setBrightness(real_t(0));
    // led0 = Color(real_t(1), real_t(1), real_t(1));e
    // auto led = AW9523::RgbLed(aw, Pin::_5, Pin::_6, Pin::_7);
    // led.setBrightness(real_t(0.2));

    while(true){
        station.run();
        if(millis() % 16 == 0)leds.refresh();
        logger.println("run");
        // if(modem.isIdle()) modem.sendCode(2);
        Sys::reCalculateTime();
    }
}
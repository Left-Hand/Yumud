#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "fwwb_compents/stations/chassis_station.hpp"

constexpr uint16_t mo_freq = 3800;

static Printer & logger = uart2;
using namespace FWWB;

void chassis_app(){

    uart2.init(115200, Uart::Mode::TxOnly);

    logger.setEps(4);

    can1.init(Can::BaudRate::Mbps1);

    auto ws_out = Gpio(GPIOB, Pin::_8);
    auto leds = WS2812Chain<3>(ws_out);
    leds.init();


    auto trigGpioA = Gpio(GPIOA, Pin::_4);;
    auto trigExtiCHA = ExtiChannel(trigGpioA, 1, 0, ExtiChannel::Trigger::RisingFalling);
    auto capA = CaptureChannelExti(trigExtiCHA, trigGpioA);
    auto demodemA = SimpleDeModem(capA, mo_freq);
    auto panelTargetA = PanelTarget(demodemA, 2, 0);

    auto panelLedA = PanelLed(leds[0]);


    auto trigGpioB = Gpio(GPIOA, Pin::_0);
    auto trigExtiCHB = ExtiChannel(trigGpioB, 1, 1, ExtiChannel::Trigger::RisingFalling);
    auto capB = CaptureChannelExti(trigExtiCHB, trigGpioB);
    auto demodemB = SimpleDeModem(capB, mo_freq);
    auto panelTargetB = PanelTarget(demodemB, 2, 1);

    auto panelLedB = PanelLed(leds[2]);

    auto panelUnitA = PanelUnit(panelTargetA, panelLedA);
    auto panelUnitB = PanelUnit(panelTargetB, panelLedB);

    auto can_station = CanStation(can1, logger);
    auto target_station = TargetStation(can_station, panelUnitA, panelUnitB);

    auto trigGpioL = Gpio(GPIOA, Pin::_5);
    auto trigGpioR = Gpio(GPIOB, Pin::_1);

    auto trigExtiCHL = ExtiChannel(trigGpioL, 1, 3, ExtiChannel::Trigger::Rising);
    auto trigExtiCHR = ExtiChannel(trigGpioR, 1, 4, ExtiChannel::Trigger::Rising);
    auto capL = CaptureChannelExti(trigExtiCHL, trigGpioL);
    auto capR = CaptureChannelExti(trigExtiCHR, trigGpioR);
    capL.init();
    capR.init();

    timer3.init(3000);
    auto & tim3ch1 = timer3[1];
    auto & tim3ch2 = timer3[2];

    auto pwmL = PwmChannel(timer3[1]);
    auto pwmR = PwmChannel(timer3[2]);

    pwmL.init();
    pwmR.init();

    tim3ch1.setPolarity(false);
    tim3ch2.setPolarity(false);

    auto dirPinL = Gpio(GPIOD, Pin::_1);
    auto dirPinR = Gpio(GPIOD, Pin::_0);

    auto motorL = GM25(pwmL, dirPinL, capL, false);
    auto motorR = GM25(pwmR, dirPinR, capR, true);
    auto station = DiffChassisStation(target_station, motorL, motorR);
    station.init();

    while(1){
        station.run();
        if(millis() % 16 == 0)leds.refresh();
        // motorL.setOmega(400 * sin(t));
        // motorR.setOmega(400 * sin(t));
        Sys::reCalculateTime();
    }
};
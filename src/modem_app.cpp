#include "apps.h"

#include "src/device/CommonIO/Led/rgbLed.hpp"
#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "fwwb_compents/stations/defense_station.hpp"

using Color = Color_t<real_t>;
Printer & logger = uart2;
constexpr uint32_t mo_freq = 3800;

using namespace Sys::Clock;
//Laser PB8 TM4CH3
//TrigIn PA4
void modem_app(){
    using namespace FWWB;

    uart2.init(115200);
    logger.setEps(4);

    can1.init(Can::BaudRate::Mbps1);
    can1.enableHwReTransmit(false);

    auto & ws_out = portB[8];
    auto leds = WS2812Chain<3>(ws_out);
    leds.init();

    auto & i2c_scl = portD[1];
    auto & i2c_sda = portD[0];
    auto i2csw = I2cSw(i2c_scl, i2c_sda);
    i2csw.init(400000);

    auto aw_drv = I2cDrv(i2csw, 0b10110000);
    auto aw = AW9523(aw_drv);
    aw.init();
    aw.setLedCurrentLimit(AW9523::CurrentLimit::Low);
    aw.enableLedMode(Pin::_8, true);
    // aw.setModeByIndex(8, PinMode::OutPP);

    auto awio = GpioVirtual(&aw, Pin::_0);
    awio.OutPP();

    auto led0 = AW9523RgbLed(aw, Pin::_2, Pin::_3, Pin::_4);
    led0.init();
    led0.setBrightness(real_t(0.1));

    auto & trigGpioA = portA[0];
    auto trigExtiCHA = ExtiChannel(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    auto capA = CaptureChannelExti(trigExtiCHA, trigGpioA);
    auto demodemA = SimpleDeModem(capA, mo_freq);
    auto panelTargetA = PanelTarget(demodemA, 2, 0);

    auto panelLedA = PanelLed(led0);


    auto & trigGpioB = portA[4];
    auto trigExtiCHB = ExtiChannel(trigGpioB, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    auto capB = CaptureChannelExti(trigExtiCHB, trigGpioB);
    auto demodemB = SimpleDeModem(capB, mo_freq);
    auto panelTargetB = PanelTarget(demodemB, 2, 1);

    auto panelLedB = PanelLed(leds[2]);

    auto panelUnitA = PanelUnit(panelTargetA, panelLedA);
    auto panelUnitB = PanelUnit(panelTargetB, panelLedB);

    CanStation can_station(can1, logger);
    can_station.init();
    TargetStation target_station(can_station, panelUnitA, panelUnitB);
    target_station.init();
    DefenseStation station(target_station);
    station.init();


    // auto led0 = AW9523::RgbLed(aw, Pin::_2, Pin::_3, Pin::_4);
    // led0.init();
    // led0.setBrightness(real_t(0));
    // led0 = Color(real_t(1), real_t(1), real_t(1));e
    // auto led = AW9523::RgbLed(aw, Pin::_5, Pin::_6, Pin::_7);
    // led.setBrightness(real_t(0.2));
        // panelLedA.setTranstit(Color(1,0,0), Color(0,1,0), PanelLed::Method::Sine);
        // panelLedA.setPeriod(1000);
    volatile uint32_t last_blink_millis = 0;
    while(true){
        if((millis() > last_blink_millis) and (millis() % 2000 == 0)){
            leds.refresh();
            last_blink_millis = millis();
            awio = !awio;
        }
        station.run();
        reCalculateTime();
    }
}
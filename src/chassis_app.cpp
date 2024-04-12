#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "src/device/VirtualIO/AW9523/aw9523.hpp"
#include "fwwb_compents/stations/chassis_station.hpp"
#include "src/system.hpp"

constexpr uint16_t mo_freq = 3800;

static Printer & logger = uart2;
using namespace FWWB;
using namespace Sys::Clock;
void chassis_app(){

    uart2.init(115200, Uart::Mode::TxOnly);
    logger.setEps(4);
    logger.setSpace(" ");
    logger.println("chassis power on");

    can1.init(Can::BaudRate::Mbps1);


    // auto ws_out = portA[-1];
    // auto leds = WS2812Chain<3>(ws_out);
    // leds.init();

    auto i2c_scl = portD[1];
    auto i2c_sda = portD[0];
    auto i2csw = I2cSw(i2c_scl, i2c_sda);
    i2csw.init(0);

    auto aw_drv = I2cDrv(i2csw, 0b10110000);
    auto aw = AW9523(aw_drv);
    aw.init();

    auto awled_r = AW9523RgbLed(aw, Pin::_11, Pin::_1, Pin::_0);
    auto awled_l = AW9523RgbLed(aw, Pin::_8, Pin::_10, Pin::_9);


    auto trigGpioA = portA[0];
    auto trigExtiCHA = ExtiChannel(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    auto capA = CaptureChannelExti(trigExtiCHA, trigGpioA);
    auto demodemA = SimpleDeModem(capA, mo_freq);
    auto panelTargetA = PanelTarget(demodemA, 2, 0);

    auto panelLedA = PanelLed(awled_l);


    auto trigGpioB = portA[4];
    auto trigExtiCHB = ExtiChannel(trigGpioB, NvicPriority(1, 1), ExtiChannel::Trigger::RisingFalling);
    auto capB = CaptureChannelExti(trigExtiCHB, trigGpioB);
    auto demodemB = SimpleDeModem(capB, mo_freq);
    auto panelTargetB = PanelTarget(demodemB, 2, 1);

    auto panelLedB = PanelLed(awled_r);

    auto panelUnitA = PanelUnit(panelTargetA, panelLedA);
    auto panelUnitB = PanelUnit(panelTargetB, panelLedB);

    auto trigGpioL = portA[5];
    auto trigGpioR = portB[1];

    auto trigExtiCHL = ExtiChannel(trigGpioL, NvicPriority(1, 3), ExtiChannel::Trigger::Rising);
    auto trigExtiCHR = ExtiChannel(trigGpioR, NvicPriority(1, 4), ExtiChannel::Trigger::Rising);
    auto capL = CaptureChannelExti(trigExtiCHL, trigGpioL);
    auto capR = CaptureChannelExti(trigExtiCHR, trigGpioR);

    timer3.init(3000);
    timer3[1].setPolarity(false);
    timer3[2].setPolarity(false);

    auto pwmL = PwmChannel(timer3[1]);
    auto pwmR = PwmChannel(timer3[2]);

    auto motorL = GM25(pwmL, portA[1], capL, false);
    auto motorR = GM25(pwmR, portB[8], capR, true);

    auto can_station = CanStation(can1, logger);
    auto target_station = TargetStation(can_station, panelUnitA, panelUnitB);
    auto station = DiffChassisStation(target_station, motorL, motorR);
    station.init();

    while(1){
        station.run();
        // logger.println(capL.getFreq(), ',', capR.getFreq());
        // while(!can1.available());
        // if(millis() % 16 == 0)leds.refresh();
        // delay(22);
        motorL.setOmega(288 * sin(t));
        motorR.setOmega(288 * cos(t));
        // motorR.setOmega(400 * sin(t));
        reCalculateTime();
    }
};
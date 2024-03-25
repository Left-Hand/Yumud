#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "src/fwwb_compents/stations/attack_station.hpp"

using namespace FWWB;

constexpr uint16_t mo_freq = 3800;

static Printer & logger = uart1;
void stepper_app(){
    uart1.init(115200 * 4);

    can1.init(Can::BaudRate::Mbps1, 1);

    spi1.init(18000000);
    spi1.bindCsPin(Gpio(GPIOA, Pin::_15), 0);

    SpiDrv mt6816_drv(spi1, 0);
    MT6816 mt6816(mt6816_drv);
    // MA730 ma730(mt6816_drv);
    // ma730.setDirection(true);

    logger.setEps(4);


    //setup servo
    timer4.init(1440,2000);
    auto tim4ch3 = timer4.getChannel(TimerOC::Channel::CH3);
    tim4ch3.init();

    auto pwmServo = PwmChannel(tim4ch3);
    auto servo = PwmAngleServo(pwmServo);
    servo.init();


    //setup svpwm
    Gpio gpioCoilAp(GPIOA, Pin::_5);
    Gpio gpioCoilAm(GPIOA, Pin::_4);
    Gpio gpioCoilBp(GPIOA, Pin::_2);
    Gpio gpioCoilBm(GPIOA, Pin::_3);

    auto tim2ch3 = timer2.getChannel(TimerOC::Channel::CH3);
    auto tim2ch4 = timer2.getChannel(TimerOC::Channel::CH4);
    PwmChannel pwmCoilA(tim2ch4);
    PwmChannel pwmCoilB(tim2ch3);

    Coil1 coilA(gpioCoilAp, gpioCoilAm, pwmCoilA);
    Coil1 coilB(gpioCoilBp, gpioCoilBm, pwmCoilB);
    SVPWM2 svpwm(coilA, coilB);

    svpwm.init();

    timer2.init(72000);

    //align motor
    auto pos_pid = PID_t<real_t>(real_t(30), real_t(), real_t(700));
    pos_pid.setClamp(real_t(0.35));
    svpwm.setABCurrent(real_t(0.8), real_t(0));
    delay(100);

    Odometer odo(mt6816,50);
    odo.locateElecrad();
    odo.locateAbsolutely(real_t(0.19));

    //setup motor
    MotorWithFoc motor(svpwm, odo, pos_pid);
    motor.setMaxCurrent(real_t(0.17));

    timer1.enableIt(Timer::IT::Update, 0, 0);
    timer1.bindCb(Timer::IT::Update, [&motor](){
        motor.closeLoop();
    });
    timer1.init(20000);

    Gpio ws_out(GPIOB, Pin::None);
    WS2812Chain<3> leds(ws_out);
    leds.init();

    Gpio trigGpioA(GPIOA, Pin::None);
    ExtiChannel trigExtiCHA(trigGpioA, 1, 0, ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capA(trigExtiCHA, trigGpioA);
    SimpleDeModem demodemA(capA, mo_freq);
    PanelTarget panelTargetA(demodemA, 2, 0);

    PanelLed panelLedA(leds[0]);

    Gpio trigGpioB(GPIOA, Pin::None);
    ExtiChannel trigExtiCHB(trigGpioB, 1, 1, ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capB(trigExtiCHB, trigGpioB);
    SimpleDeModem demodemB(capB, mo_freq);
    PanelTarget panelTargetB(demodemB, 2, 1);

    PanelLed panelLedB(leds[2]);

    PanelUnit panelUnitA(panelTargetA, panelLedA);
    PanelUnit panelUnitB(panelTargetB, panelLedB);

    timer3.init(38000);
    auto tim3ch2 = timer3.getChannel(TimerOC::Channel::CH2);
    tim3ch2.init();

    auto pwm_out = PwmChannel(tim3ch2);

    auto modem = SimpleModem(pwm_out);
    timer3.enableIt(Timer::IT::Update, 0, 1);
    timer3.bindCb(Timer::IT::Update, [&modem](){
        static uint8_t cycle = 0;
        if(cycle == 9){
            modem.tick();
            cycle = 0;
        }
    });

    CanStation can_station(can1, logger);
    TargetStation target_station(can_station, panelUnitA, panelUnitB);
    AttackStation attack_station(target_station, motor, servo, modem);
    attack_station.init();

    while(true){
        // uart1.println(motor.getPosition(), target);
        // delayMicroseconds(500);
        // // target = floor(t*16)/4 + 0.125;
        // target = real_t(0.015*cos(4 *t));

        // // target = real_t(0);
        // // target = 6 * sin(t);
        // servo.setAngle(4* sin(4*t) + 92);
        attack_station.run();
        Sys::reCalculateTime();

    }
}
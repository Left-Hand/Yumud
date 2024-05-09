#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "src/fwwb_compents/stations/attack_station.hpp"
#include "src/device/Encoder/Odometer.hpp"

using namespace FWWB;
using namespace Sys::Clock;
using namespace Sys::Chip;


constexpr uint16_t mo_freq = 3800;
static Printer & logger = uart1;



void stepper_app_new(){
    uart1.init(115200);
    logger.setEps(4);

    can1.init(Can::BaudRate::Mbps1, 1);

    spi1.init(18000000);
    spi1.bindCsPin(portA[15], 0);

    timer3.init(38000);

    PwmChannel pwmCoilA(timer3[3]);
    PwmChannel pwmCoilB(timer3[2]);

    Coil1 coilA(portA[10], portA[11], timer3[2]);
    Coil1 coilB(portA[8], portA[9], timer3[3]);
    SVPWM2 svpwm(coilA, coilB);
    svpwm.init();

    SpiDrv mt6816_drv(spi1, 0);
    MT6816 mt6816(mt6816_drv);
    auto odo = OdometerPoles(mt6816);
    odo.init();

    auto pos_pid = PID_t<real_t>(real_t(10), real_t(), real_t(0));
    pos_pid.setClamp(real_t(0.35));

    MotorWithFoc motor(svpwm, odo, pos_pid);
    motor.setMaxCurrent(real_t(0.15));

    auto modem_pwm_out = PwmChannel(timer3[1]);
    auto modem = SimpleModem(modem_pwm_out);
    modem.init();

    timer2.init(14400, 200);

    auto servo_pwm = PwmChannel(timer2[1]);
    timer2[1].init();
    auto servo = PwmAngleServo(servo_pwm, -180);
    servo.init();


    WS2812Chain<2> leds(portB[1]);
    leds.init();

    Gpio & trigGpioA = portA[2];
    ExtiChannel trigExtiCHA(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capA(trigExtiCHA, trigGpioA);
    SimpleDeModem demodemA(capA, mo_freq);
    PanelTarget panelTargetA(demodemA, 2, 0);

    PanelLed panelLedA(leds[0]);

    Gpio & trigGpioB = portA[1];
    ExtiChannel trigExtiCHB(trigGpioB, NvicPriority(1, 1), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capB(trigExtiCHB, trigGpioB);
    SimpleDeModem demodemB(capB, mo_freq);
    PanelTarget panelTargetB(demodemB, 2, 1);

    PanelLed panelLedB(leds[-1]);

    PanelUnit panelUnitA(panelTargetA, panelLedA);
    PanelUnit panelUnitB(panelTargetB, panelLedB);

    CanStation can_station(can1, logger);
    can_station.init();
    TargetStation target_station(can_station, panelUnitA, panelUnitB);
    target_station.init();
    AttackStation station(target_station, motor, servo, modem);
    station.init();

    auto & bled = portC[13];
    bled.OutPP();

    uint8_t motor_code = 0;

    logger.println(getChipIdCrc());
    switch(getChipIdCrc()){
        case 3273134334:
            motor_code = 1;
            break;
        case 181345611:
            motor_code = 0;
    }
    // logger.println(getChipIdCrc());

    if(motor_code == 0){
        odo.inverse(false);
    }else{
        odo.inverse(false);
    }
    // 
    // odo.inverse(false);
    // svpwm.inverse(true);
    // svpwm.inverse(false);
    odo.reset();
    {
        logger.println("Cali..");

        constexpr int forwardpreturns = 1;
        constexpr int forwardturns = 3;
        constexpr int backwardpreturns = 1;
        constexpr int backwardturns = 3;
        constexpr int turnmircos = 256;
        constexpr int dur = 600;
        constexpr float cali_current = 0.6;

        for(int i = -forwardpreturns * turnmircos; i < forwardturns * turnmircos; i++){

            if(i >= 0 && i % turnmircos == 0){//measureable
                // odo.locateElecrad(real_t(1.0 / (forwardturns + backwardturns)));
            }
            real_t elecrad = i * real_t((TAU / turnmircos));
            svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
            delayMicroseconds(dur);
        }

        for(int i = -backwardpreturns * turnmircos; i < backwardturns * turnmircos; i++){

            if(i >= 0 && i % turnmircos == 0){//measureable
                // odo.locateElecrad(real_t(1.0 / (forwardturns + backwardturns)));
            }
            real_t elecrad = -i * real_t((TAU / turnmircos));
            svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
            delayMicroseconds(dur);
        }

        logger.println("Cali done");
    }

    if(motor_code == 1){
        odo.locateAbsolutely(real_t(-0.01));
    }else{
        odo.locateAbsolutely(real_t(0.47));
    }
    svpwm.setDQCurrent(Vector2(), real_t(0));
    // motor.enable(false);
    // motor.open();

    timer4.init(38000);
    timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));
    timer4.bindCb(Timer::IT::Update, [&motor, &modem](){
        motor.run();
        static uint8_t cycle = 0;
        cycle++;
        if(cycle == 10){
            modem.tick();
            cycle = 0;
        }
    });

    volatile uint32_t last_blink_millis = 0;
    while(true){
        if((millis() > last_blink_millis) and (millis() % 16 == 0)){
            bled= !bled;
            leds.refresh();
            last_blink_millis = millis();
        }

        station.run();
        // logger.println(odo.getPosition());
        // station.setFace(Vector2(real_t(0.2), real_t(0)).rotate(t));
        // station.setShotCode(2);
        // station.shotMs(65535);
        // station.setFace(Vector2(real_t(0.1), real_t(0)).rotate(t));
        // station.setYaw(sin(t));
        // motor.run();
        // logger.println(int(timer3[2]), int(timer3[3]));
        // leds[0] = Color_t<real_t>(1, 0, 0, 1);
        // leds.refresh();
        // if(modem.isIdle())modem.sendCode(2);

        // motor.enable(false);

        // odo.update();
        // motor.trackLapPos(sin(t) * 0);
        // motor.trackLapPos(real_t(0));
        // motor.trackCurr(real_t(0.1));
        // odo.update();
        // real_t elecrad = odo.getElecRad();
        // real_t elecrad = odo.getElecRad();
        // odo.update();
        // logger.println(odo.getElecRad(), fmod(elecrad, real_t(TAU)));
        // svpwm.setABCurrent(0.14 * sin(elecrad), 0.14 * cos(elecrad));
        // svpwm.setDQCurrent(real_t(0), real_t(0.08), elecrad);
        // servo.setDuty(0.5 + 0.5 * sin(t));
        reCalculateTime();
    }
}
#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "src/fwwb_compents/stations/attack_station.hpp"

using namespace FWWB;

constexpr uint16_t mo_freq = 3800;
static Printer & logger = uart1;

void stepper_app(){
    uart1.init(115200);

    can1.init(Can::BaudRate::Mbps1, 1);

    spi1.init(18000000);
    spi1.bindCsPin(Gpio(GPIOA, Pin::_15), 0);

    SpiDrv mt6816_drv(spi1, 0);
    MT6816 mt6816(mt6816_drv);
    // MA730 ma730(mt6816_drv);
    // ma730.setDirection(true);

    logger.setEps(4);

    //setup svpwm
    // portA.getInstance

    timer2.init(72000);
    PwmChannel pwmCoilA(timer2[4]);
    PwmChannel pwmCoilB(timer2[3]);

    Coil1 coilA(portA[5], portA[4], pwmCoilA);
    Coil1 coilB(portA[2], portA[3], pwmCoilB);
    SVPWM2 svpwm(coilA, coilB);

    svpwm.init();
    svpwm.setABCurrent(real_t(0.8), real_t(0));
    delay(100);
    Odometer odo(mt6816,50);
    odo.locateElecrad();
    odo.locateAbsolutely(real_t(0.19));
    svpwm.setABCurrent(real_t(0), real_t(0));


    // timer3.init(50);
    // uint32_t raw_period = 144000000 / 50;
    // uint16_t cycle = 1;
    // while(raw_period / cycle > 16384){
    //     cycle++;
    // }

    // timer3.init(raw_period / cycle, cycle);
    timer3.init(14400, 200);
    // timer3.enableIt

    // if(Sys::getChipIdCrc() == 4127488304){
    // // logger.println(1);
    // volatile String str = String(1);
    // // uart1.println(str);
    // // USART1->DATAR = '1';
    // // delay(1);
    // // USART1->DATAR = '\r';
    // // delay(1);
    // // USART1->DATAR = '\n';
    // }
    // logger.println(Sys::getChipIdCrc());
    // logger.println("hihigyuhkoihojiojo");
    // timer3.init(raw_period / cycle,cycle);
    // timer3.init(0, 18);

    auto servo_pwm = PwmChannel(timer3[2]);
    auto servo = PwmAngleServo(servo_pwm, -180);
    servo.init();


    //align motor
    auto pos_pid = PID_t<real_t>(real_t(10), real_t(), real_t(700));
    pos_pid.setClamp(real_t(0.35));
    //setup motor
    MotorWithFoc motor(svpwm, odo, pos_pid);
    motor.setMaxCurrent(real_t(0.05));

    timer1.init(38000);


    auto modem_pwm_out = PwmChannel(timer1[1]);
    auto modem = SimpleModem(modem_pwm_out);
    modem.init();

    timer1.enableIt(Timer::IT::Update, 0, 0);
    timer1.bindCb(Timer::IT::Update, [&motor, & modem](){
        motor.closeLoop();
        static uint8_t cycle = 0;
        cycle++;
        if(cycle == 10){
            modem.tick();
            cycle = 0;
        }
    });

    WS2812Chain<3> leds(portB[-1]);
    leds.init();

    Gpio trigGpioA(GPIOA, Pin::_9);
    ExtiChannel trigExtiCHA(trigGpioA, 1, 0, ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capA(trigExtiCHA, trigGpioA);
    SimpleDeModem demodemA(capA, mo_freq);
    PanelTarget panelTargetA(demodemA, 2, 0);

    PanelLed panelLedA(leds[0]);

    Gpio trigGpioB(GPIOB, Pin::_12);
    ExtiChannel trigExtiCHB(trigGpioB, 1, 1, ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capB(trigExtiCHB, trigGpioB);
    SimpleDeModem demodemB(capB, mo_freq);
    PanelTarget panelTargetB(demodemB, 2, 1);

    PanelLed panelLedB(leds[2]);

    PanelUnit panelUnitA(panelTargetA, panelLedA);
    PanelUnit panelUnitB(panelTargetB, panelLedB);

    CanStation can_station(can1, logger);
    can_station.init();
    TargetStation target_station(can_station, panelUnitA, panelUnitB);
    target_station.init();
    AttackStation attack_station(target_station, motor, servo, modem);
    attack_station.init();

    Gpio bled(GPIOC, Pin::_13);
    bled.OutPP();
    volatile uint32_t last_blink_millis = 0;
    while(true){
        if((millis() > last_blink_millis) and (millis() % 200 == 0)){
            bled= !bled;
            last_blink_millis = millis();
        }

        // delayMicroseconds(500);
        // target = floor(t*16)/4 + 0.125;
        // real_t target = real_t(0.015*cos(4 *t));
        // logger.println(TIM3->CH2CVR);

        // motor.trackLapPos(0.05*sin(t));
        // logger.println(target);
        // target = real_t(0);
        // target = 6 * sin(t);
        // servo.setAngle(4* sin(4*t) + 92);
// servo.setAngle(4* sin(t) + 92);
        // modem.tick();
        // if(modem.isIdle())
        // attack_station.setFace(Vector2(real_t(0.2), real_t(0)).rotate(real_t(PI / 4) + real_t(PI / 2) * int(t)));
        attack_station.run();
        // can1.write(CanMsg((uint8_t)(Command::ACTIVE) << 4, true));
        // target_station.run();

        Sys::reCalculateTime();
    }
}


void stepper_app_new(){
    uart1.init(115200);

    can1.init(Can::BaudRate::Mbps1, 1);

    spi1.init(18000000);
    spi1.bindCsPin(Gpio(GPIOA, Pin::_15), 0);

    SpiDrv mt6816_drv(spi1, 0);
    MT6816 mt6816(mt6816_drv);
    // MA730 ma730(mt6816_drv);
    // ma730.setDirection(true);

    logger.setEps(4);


    timer3.init(38000);

    // auto servo_pwm = PwmChannel(tim3ch1);
    PwmChannel pwmCoilA(timer3[3]);
    PwmChannel pwmCoilB(timer3[2]);

    Coil1 coilA(portA[10], portA[11], pwmCoilA);
    Coil1 coilB(portA[8], portA[9], pwmCoilB);
    SVPWM2 svpwm(coilA, coilB);

    auto modem_pwm_out = PwmChannel(timer3[1]);
    auto modem = SimpleModem(modem_pwm_out);
    modem.init();

    svpwm.init();
    svpwm.setABCurrent(real_t(0.8), real_t(0));
    delay(100);
    Odometer odo(mt6816,50);
    odo.locateElecrad();
    odo.locateAbsolutely(real_t(0.19));
    svpwm.setABCurrent(real_t(0), real_t(0));
    // while(true){
    //     svpwm.setABCurrent(cos(200*t)*0.2, sin(200*t)*0.2);
    //     odo.update();
    //     logger.println(odo.getPosition().value, mt6816.getPositionData());
    //     Sys::reCalculateTime();
    // }





    // timer3.init(50);
    // uint32_t raw_period = 144000000 / 50;
    // uint16_t cycle = 1;
    // while(raw_period / cycle > 16384){
    //     cycle++;
    // }

    // timer3.init(raw_period / cycle, cycle);

    timer2.init(14400, 200);
    // timer3.enableIt

    // if(Sys::getChipIdCrc() == 4127488304){
    // // logger.println(1);
    // volatile String str = String(1);
    // // uart1.println(str);
    // // USART1->DATAR = '1';
    // // delay(1);
    // // USART1->DATAR = '\r';
    // // delay(1);
    // // USART1->DATAR = '\n';
    // }
    // logger.println(Sys::getChipIdCrc());
    // logger.println("hihigyuhkoihojiojo");
    // timer3.init(raw_period / cycle,cycle);
    // timer3.init(0, 18);

    auto servo_pwm = PwmChannel(timer2[1]);
    auto servo = PwmAngleServo(servo_pwm, -180);
    servo.init();


    //align motor
    auto pos_pid = PID_t<real_t>(real_t(10), real_t(), real_t(700));
    pos_pid.setClamp(real_t(0.35));
    //setup motor
    MotorWithFoc motor(svpwm, odo, pos_pid);
    motor.setMaxCurrent(real_t(0.15));


    Gpio ws_out(GPIOB, Pin::_1);
    WS2812Chain<3> leds(ws_out);
    leds.init();

    Gpio trigGpioA(GPIOA, Pin::_2);
    ExtiChannel trigExtiCHA(trigGpioA, 1, 0, ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capA(trigExtiCHA, trigGpioA);
    SimpleDeModem demodemA(capA, mo_freq);
    PanelTarget panelTargetA(demodemA, 2, 0);

    PanelLed panelLedA(leds[0]);

    Gpio trigGpioB(GPIOA, Pin::_1);
    ExtiChannel trigExtiCHB(trigGpioB, 1, 1, ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti capB(trigExtiCHB, trigGpioB);
    SimpleDeModem demodemB(capB, mo_freq);
    PanelTarget panelTargetB(demodemB, 2, 1);

    PanelLed panelLedB(leds[2]);

    PanelUnit panelUnitA(panelTargetA, panelLedA);
    PanelUnit panelUnitB(panelTargetB, panelLedB);

    CanStation can_station(can1, logger);
    can_station.init();
    TargetStation target_station(can_station, panelUnitA, panelUnitB);
    target_station.init();
    AttackStation attack_station(target_station, motor, servo, modem);
    attack_station.init();

    timer3.enableIt(Timer::IT::Update, 0, 0);
    timer3.bindCb(Timer::IT::Update, [&motor, &modem](){
        motor.closeLoop();
        static uint8_t cycle = 0;
        cycle++;
        if(cycle == 10){
            modem.tick();
            cycle = 0;
        }
    });

    Gpio bled(GPIOC, Pin::_13);
    bled.OutPP();
    volatile uint32_t last_blink_millis = 0;
    while(true){
        if((millis() > last_blink_millis) and (millis() % 200 == 0)){
            bled= !bled;
            last_blink_millis = millis();
        }

        // attack_station.shotMs(200);
        if(modem.isIdle())modem.sendCode(3);
        // modem_pwm_out = real_t(0.1);
        // TIM3->CH1CVR = TIM3->ATRLR >> 3;
        logger.println(TIM3->CH1CVR);
        // logger.println(odo.getPosition().toString());
        // delayMicroseconds(500);
        // target = floor(t*16)/4 + 0.125;
        // real_t target = real_t(0.015*cos(4 *t));
        // logger.println(TIM3->CH2CVR);
        // odo.update();
        // motor.closeLoop();
        motor.trackLapPos(0.2*sin(t));
        // logger.println(int(mt6816.getPosition()));
        // logger.println(target);
        // target = real_t(0);
        // target = 6 * sin(t);
        // servo.setAngle(4* sin(4*t) + 92);
// servo.setAngle(4* sin(t) + 92);
        // modem.tick();
        // if(modem.isIdle())
        // attack_station.setFace(Vector2(real_t(0.2), real_t(0)).rotate(real_t(PI / 4) + real_t(PI / 2) * int(t)));
        // attack_station.run();
        // can1.write(CanMsg((uint8_t)(Command::ACTIVE) << 4, true));
        // target_station.run();

        Sys::reCalculateTime();
    }
}
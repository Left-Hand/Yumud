#include "apps.h"

void stepper_app(){
    uart1.init(115200 *4);
    spi1.init(18000000);
    spi1.bindCsPin(Gpio(GPIOA, Pin::_15), 0);
    SpiDrv mt6816_drv(spi1, 0);
    MT6816 mt6816(mt6816_drv);
    // MA730 ma730(mt6816_drv);
    // ma730.setDirection(true);
    Printer & log = uart1;
    log.setEps(4);

    auto tim2ch3 = timer2.getChannel(TimerOC::Channel::CH3);
    auto tim2ch4 = timer2.getChannel(TimerOC::Channel::CH4);

    timer4.init(1440,2000);
    auto tim4ch3 = timer4.getChannel(TimerOC::Channel::CH3);
    tim4ch3.init();

    auto pwmServo = PwmChannel(tim4ch3);
    auto servo = PwmAngleServo(pwmServo);
    servo.init();
    Gpio gpioCoilAp = Gpio(GPIOA, Pin::_5);
    Gpio gpioCoilAm = Gpio(GPIOA, Pin::_4);
    Gpio gpioCoilBp = Gpio(GPIOA, Pin::_2);
    Gpio gpioCoilBm = Gpio(GPIOA, Pin::_3);
    auto pwmCoilA = PwmChannel(tim2ch4);
    auto pwmCoilB = PwmChannel(tim2ch3);

    timer2.init(72000);

    uint32_t nanos0 = 0;
    uint32_t nanos1 = 0;
    real_t target;

    timer1.init(20000);

    Coil1 coilA(gpioCoilAp, gpioCoilAm, pwmCoilA);
    Coil1 coilB(gpioCoilBp, gpioCoilBm, pwmCoilB);
    SVPWM2 svpwm(coilA, coilB);

    svpwm.init();
    auto pos_pid = PID_t<real_t>(real_t(30), real_t(), real_t(700));
    // auto pos_pid = PID_t<real_t>(real_t(6), real_t(0.003), real_t(7));
    pos_pid.setClamp(real_t(0.35));
    svpwm.setABCurrent(real_t(0.8), real_t(0));
    delay(100);

    Odometer odo(mt6816,50);
    odo.locateElecrad();
    odo.locateAbsolutely(real_t(0.19));

    MotorWithFoc motor(svpwm, odo, pos_pid);
    motor.setMaxCurrent(real_t(0.17));

    timer1.enableIt(Timer::IT::Update);
    timer1.bindCb(Timer::IT::Update, [&motor, &target](){
        motor.closeLoopPos(target);
    });

    while(true){
        uart1.println(motor.getPosition(), target);
        delayMicroseconds(500);
        // target = floor(t*16)/4 + 0.125;
        target = real_t(0.015*cos(4 *t));

        // target = real_t(0);
        // target = 6 * sin(t);
        servo.setAngle(4* sin(4*t) + 92);
        reCalculateTime();

    }
}
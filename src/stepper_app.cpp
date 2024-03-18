#include "apps.h"

void stepper_app(){
    uart1.init(115200 *4);
    spi1.init(18000000);
    spi1.bindCsPin(Gpio(GPIOA, Pin::_15), 0);
    SpiDrv mt6816_drv(spi1, 0);
    MT6816 mt6816(mt6816_drv);
    Printer & log = uart1;
    log.setEps(4);

    auto tim2ch3 = timer2.getChannel(TimerOC::Channel::CH3);
    auto tim2ch4 = timer2.getChannel(TimerOC::Channel::CH4);

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
    auto pos_pid = PID_t<real_t>(real_t(6), real_t(), real_t(700));

    pos_pid.setClamp(real_t(0.35));
    svpwm.setABCurrent(real_t(0.8), real_t(0));
    delay(100);

    Odometer odo(mt6816,50);
    odo.locateElecrad();
    odo.locateAbsolutely(real_t(-0.5));

    MotorWithFoc motor(svpwm, odo, pos_pid);
    motor.setMaxCurrent(real_t(0.13));

    timer1.enableIt(Timer::IT::Update);
    timer1.bindCb(Timer::IT::Update, [&motor, &target, &nanos1, &nanos0](){
        nanos1 = micros();
        motor.closeLoopPos(target);
        nanos0 = micros();
    });

    while(true){
        uart1.println(motor.getPosition(), target, nanos1, nanos0);
        delay(1);
        // target = floor(t*16)/4 + 0.125;
        target = real_t(sin(t));
        // target = 6 * sin(t);
        reCalculateTime();

    }
}
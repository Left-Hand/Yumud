#include "apps.h"

void stepper_app(){
    uart1.init(115200 *4);
    spi1.init(9000000);
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
    real_t _t;
    real_t target;
    timer2.bindCb(Timer::IT::Update, [&_t, &target](){target = 3 * sin(_t * 3);});

    Coil1 coilA(gpioCoilAp, gpioCoilAm, pwmCoilA);
    Coil1 coilB(gpioCoilBp, gpioCoilBm, pwmCoilB);
    SVPWM2 svpwm(coilA, coilB);

    svpwm.init();
    auto pos_pid = PID_t<real_t>(real_t(2), real_t(), real_t());
    pos_pid.setClamp(real_t(0.35));
    svpwm.setABCurrent(real_t(0.8), real_t(0));
    delay(100);

    Odometer odo(mt6816,50);
    odo.locateElecrad();
    odo.locateRelatively();
    // odo.locateAbsolutely();

    MotorWithFoc motor(svpwm, odo, pos_pid);
    motor.setMaxCurrent(real_t(0.13));

    while(true){
        motor.closeLoopPos(target);
        // motor.closeLoopPos(round(odo.getPosition() * 7)/7);
        _t = t;
        reCalculateTime();
    }
}
#include "node.hpp"
#include "robots/stepper/stepper.hpp"
#include "drivers/Actuator/Driver/AT8222/at8222.hpp"

void node_main(){
    using TimerUtils::Mode;
    using TimerUtils::IT;

    DEBUGGER.init(DEBUG_UART_BAUD);
    DEBUGGER.setEps(4);

    auto & logger = DEBUGGER;

    auto & ena_gpio = portB[0];
    auto & enb_gpio = portA[7];

    AT8222 coilA{timer1.oc(3), timer1.oc(4), ena_gpio};
    AT8222 coilB{timer1.oc(1), timer1.oc(2), enb_gpio};


    SVPWM2 svpwm{coilA, coilB};
    svpwm.inverse(false);
    timer1.init(chopper_freq, Mode::CenterAlignedDownTrig);
    timer1.enableArrSync();
    timer1.oc(1).init();
    timer1.oc(2).init();
    timer1.oc(3).init();
    timer1.oc(4).init();

    ena_gpio.outpp(1);
    enb_gpio.outpp(1);

    svpwm.init();

    spi1.init(18000000);
    spi1.bindCsPin(portA[15], 0);

    SpiDrv mt6816_drv{spi1, 0};
    MT6816 mt6816{mt6816_drv};

    I2cSw i2cSw{portD[1], portD[0]};
    i2cSw.init(400000);
    AT24C02 at24{i2cSw};
    Memory mem{at24};

    FOCStepper stp{logger, can1, svpwm, mt6816, mem};

    timer3.init(foc_freq, Mode::CenterAlignedDownTrig);
    timer3.enableArrSync();
    timer3.bindCb(IT::Update, [&](){stp.tick();});
    timer3.enableIt(IT::Update,{0,0});

    can1.init(Can::BaudRate::Mbps1);
 
    stp.init();
    stp.setSpeedLimit(30);
    stp.setAccelLimit(20);
    stp.setOpenLoopCurrent(real_t(0.5));
    stp.setCurrentLimit(real_t(0.4));

    while(!stp.isActive());
    stp.setTargetCurrent(real_t(0));
    stp.setCurrentLimit(real_t(1.6));

    while(true){
        stp.run(); 
        stp.report();
        // Sys::Clock::reCalculateTime();
        // stp.setTargetPosition(4 * sin(t * 2) + sign(sin(t * 2)));
        // stp.setTargetPosition(3 * sin(10*t)*(cos(t/2)));
        // stp.setTargetPosition(5 *sin(t) * sin(t*9));
        // stp.setTargetPosition(10 * int(7 * 6sin(t / 2)));
        // stp.setTargetPosition(500 * abs(frac(t/2) - 0.5));
        // stp.setTargetPosition(round(stp.getPosition() * 100)/100);
        // stp.setTargetPosition(10 * sign(sin(t * 3)));
        // stp.setTargetPosition(0.07 * sin(4 * t));
        // stp.setTargetSpeed(CLAMP(60 * sin(t * 3), 0, 30));
        // stp.setTargetTrapezoid(7 * sin(t));
        // stp.setTargetTrapezoid(10 * sign(sin(1.5 * t)));
        // stp.setTargetPosition(10 * sign(sin(1.5 * t)));
        // stp.setTargetPosition(40 * sin(t/2) + 20 * sign(sin(t/2)));
        // stp.setTargetPosition(0.3 * sin(t));
        // stp.setTargetVector(sin(t) * 0.5);
        // stp.setTargetPosition(4 * sign(sin(4 * t)));
    }
}
#include "node.hpp"
#include "robots/foc/stepper/stepper.hpp"
#include "algo/interpolation/cubic.hpp"
#include "drivers/Actuator/Driver/AT8222/at8222.hpp"
#include "algo/interpolation/cubic.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/can/can.hpp"


#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/Encoder/MagEnc/MT6701/mt6701.hpp" 
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Wireless/Radio/LT8920/lt8920.hpp"
#include "drivers/Wireless/Radio/XL2400/xl2400.hpp"


void node_main(){
    using TimerUtils::Mode;
    using TimerUtils::IT;

    DEBUGGER.init(DEBUG_UART_BAUD);
    DEBUGGER.setEps(4);

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


    FOCStepper stp{svpwm, mt6816, mem};
    AsciiProtocol ascii_p{stp, DEBUGGER};
    CanProtocol can_p{stp, can1};

    stp.bindProtocol(ascii_p);
    stp.bindProtocol(can_p);

    timer3.init(foc_freq, Mode::CenterAlignedDownTrig);
    timer3.enableArrSync();
    timer3.bindCb(IT::Update, [&](){stp.tick();});
    timer3.enableIt(IT::Update,{0,0});

    can1.init(Can::BaudRate::_1M);
 
    stp.init();
    stp.setSpeedLimit(46);
    stp.setAccelLimit(72);
    stp.setOpenLoopCurrent(real_t(0.7));
    stp.setCurrentLimit(real_t(0.4));

    while(!stp.isActive());
    stp.setTargetCurrent(real_t(0));
    stp.setCurrentLimit(real_t(1.5));

    // CubicInterpolation cubic;


    // cubic.mapping
    // t = 0;
    while(true){
        stp.run(); 
        stp.report();

        // auto f = [](const real_t x){return (x > 0) ? (x > real_t(0.2)) ? real_t(0.2) * x - real_t(0.04) : x * x : 0;};
        // real_t target = f(t-2);
        // real_t target = t * 0;
        // stp.setTargetPosition(target);
        // stp.setTargetSpeed(5);
        // stp.setTargetVector(target);

        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(stp.getPositionErr(),fmod(t, real_t(1.00)), target, stp.getPosition());
        // Sys::Clock::reCalculateTime();

        // stp.setTargetPosition(Interpolation::demo() * 10);
        // stp.setTargetPosition(5 * sin(7 * t));
        // stp.setTargetPosition(demo() * 17);
        // stp.setTargetPosition(17* sin(2 * t));
        // stp.setTargetPosition(17* sin(2 * t));
        // stp.setTargetPosition(7 * frac(t));

        // stp.setTargetPosition(0);
        // stp.setTargetPosition(abs(frac(t)-real_t(0.5)));
        // stp.setTargetPosition(CLAMP(sin(3 * t), real_t(-0.5), real_t(0.5)));
        // Sys::Clock::reCalculateTime();
        // stp.setTargetPosition(4 * sin(t * 2) + sign(sin(t * 2)));
        // stp.setTargetPosition(3 * sin(10*t)*(cos(t/2)));
        // stp.setTargetPosition(5 *sin(t) * sin(t*5));
        // stp.setTargetPosition(frac(t));
        // stp.setTargetPosition(24 * sin(2 * t));
        // stp.setTargetSpeed(24 * sin(7 * t));
        // stp.setTargetPosition(10 * int(7 * 6sin(t / 2)));
        // stp.setTargetPosition(3 * abs(frac(t*2) - real_t(0.5)));

        // stp.setTargetPosition(round(stp.getPosition() * 100)/100);
        // stp.setTargetPosition(10 * sign(sin(t * 3)));
        // stp.setTargetPosition(real_t(1.5) * sin(3 * t));
        // stp.setTargetSpeed(CLAMP(60 * sin(t * 3), 0, 30));
        // stp.setTargetTrapezoid(10 * sign(sin(3* t)));
        // stp.setTargetTrapezoid(10 * sign(sin(1.5 * t)));
        // stp.setTargetPosition(10 * sign(sin(1.5 * t)));
        // stp.setTargetPosition(40 * sin(t/2) + 20 * sign(sin(t/2)));
        // stp.setTargetPosition(real_t(0.3) * sin(t));
        // stp.setTargetVector(sin(t) * 0.5);
        // stp.setTargetPosition(4 * sign(sin(4 * t)));
    }
}
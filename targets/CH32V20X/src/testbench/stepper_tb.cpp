#include "tb.h"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/adc/adcs/adc1.hpp"

#include "robots/foc/stepper/stepper.hpp"
#include "algo/interpolation/cubic.hpp"
#include "algo/interpolation/cubic.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/Encoder/MagEnc/MT6701/mt6701.hpp" 
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"

#include "drivers/Actuator/Driver/AT8222/at8222.hpp"
#include "drivers/Actuator/Driver/MP6540/mp6540.hpp"

#include "drivers/Actuator/SVPWM/svpwm2.hpp"
#include "drivers/Actuator/SVPWM/svpwm3.hpp"

#define MOTOR_TYPE_STEPPER 0
#define MOTOR_TYPE_BLDC 1
#define MOTOR_TYPE MOTOR_TYPE_STEPPER

void stepper_tb(UartHw & logger){
    using TimerUtils::Mode;
    using TimerUtils::IT;

    logger.init(576000);
    logger.setEps(4);

    #if(MOTOR_TYPE == MOTOR_TYPE_STEPPER)
    auto & ena_gpio = portB[0];
    auto & enb_gpio = portA[7];

    AT8222 coilA{timer1.oc(3), timer1.oc(4), ena_gpio};
    AT8222 coilB{timer1.oc(1), timer1.oc(2), enb_gpio};

    SVPWM2 svpwm{coilA, coilB};

    ena_gpio.outpp(1);
    enb_gpio.outpp(1);

    #endif


    #if(MOTOR_TYPE == MOTOR_TYPE_BLDC)

    MP6540 mp6540{
        {timer1.oc(1), timer1.oc(2), timer1.oc(3)},
        {adc1.inj(1), adc1.inj(2), adc1.inj(3)}
    };

    mp6540.init();
    mp6540.setSoRes(1_K);
    
    SVPWM3 svpwm {mp6540};
    #endif
    
    svpwm.inverse(false);
    
    timer1.init(chopper_freq, Mode::CenterAlignedDownTrig);
    timer1.enableArrSync();
    timer1.oc(1).init();
    timer1.oc(2).init();
    timer1.oc(3).init();
    timer1.oc(4).init();
    

    using AdcChannelEnum = AdcUtils::Channel;
    using AdcCycleEnum = AdcUtils::SampleCycles;

    adc1.init(
        {
            AdcChannelConfig{AdcChannelEnum::VREF, AdcCycleEnum::T28_5}
        },{
            AdcChannelConfig{AdcChannelEnum::CH4, AdcCycleEnum::T7_5},
            AdcChannelConfig{AdcChannelEnum::CH3, AdcCycleEnum::T7_5},
        }
    );

    timer1.setTrgoSource(TimerUtils::TrgoSource::Update);
    timer1.setRepeatTimes(1);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1TRGO);
    adc1.enableAutoInject(false);


    svpwm.init();
    svpwm.enable();

    spi1.init(18000000);
    spi1.bindCsPin(portA[15], 0);

    SpiDrv mt6816_drv{spi1, 0};
    MT6816 mt6816{mt6816_drv};

    I2cSw i2cSw{portD[1], portD[0]};
    i2cSw.init(400000);
    AT24C02 at24{i2cSw};
    Memory mem{at24};


    FOCStepper stp{svpwm, mt6816, mem};
    AsciiProtocol ascii_p{stp, logger};
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
    stp.setAccelLimit(172);
    stp.setOpenLoopCurrent(real_t(0.7));
    stp.setCurrentLimit(real_t(0.4));

    while(!stp.isActive());
    stp.setTargetCurrent(real_t(0));
    stp.setCurrentLimit(real_t(1.8));

    // CubicInterpolation cubic;

    // cubic.mapping
    // t = 0;
    while(true){
        stp.run(); 
        stp.report();

        // auto f = [](const real_t x){return (x > 0) ? (x > real_t(0.2)) ? real_t(0.2) * x - real_t(0.04) : x * x : 0;};
        // real_t target = f(t-2);
        // real_t target = real_t(0.01) * t;
        // stp.setTargetPosition(target);
        // stp.setTargetVector(target);
        // stp.setTargetSpeed(5);
        // stp.setTargetVector(target);

        // if(logger.pending() == 0) logger.println(stp.getPositionErr(),fmod(t, real_t(1.00)), target, stp.getPosition());
        // if(logger.pending() == 0) logger.println(real_t(adc1.inj(1)), real_t(adc1.inj(2)));
        // Sys::Clock::reCalculateTime();

        // stp.setTargetPosition(Interpolation::demo() * 10);
        // stp.setTargetPosition(5 * sin(7 * t));
        // stp.setTargetPosition(demo() * 17);

        // stp.setTargetPosition(17* sin(2 * t));
        // stp.setTargetPosition(7 * frac(t));

        // real_t target = real_t(0.7) * sin(t);
        // stp.setTargetPosition(target);
        // stp.setTargetCurrent(target);
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN(stp.getPosition(), target);

        // real_t target = real_t(20.4) * sin(7 * t);
        real_t target = real_t(30.4) * sign(sin(5 * t));
        // real_t target = real_t(0.8) * sign(sin(5 * t));
        stp.setTargetSpeed(target);
        // stp.setTargetCurrent(target);
        if(DEBUGGER.pending() == 0)DEBUG_PRINTLN(stp.getSpeed(), stp.getTarget());
        
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
        Sys::Clock::reCalculateTime();
    }
}
#include "tb.h"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/adc/adcs/adc1.hpp"

#include "robots/foc/stepper/stepper.hpp"
#include "algo/interpolation/cubic.hpp"
#include "algo/interpolation/cubic.hpp"

#include "drivers/Memory/EEprom/AT24CXX/at24cxx.hpp"


#include "drivers/Encoder/MagEncoder.hpp"
#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/Encoder/MagEnc/MT6701/mt6701.hpp" 
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"

#include "drivers/Actuator/Driver/AT8222/at8222.hpp"
#include "drivers/Actuator/Driver/MP6540/mp6540.hpp"

#include "drivers/Actuator/SVPWM/svpwm2.hpp"
#include "drivers/Actuator/SVPWM/svpwm3.hpp"

#include "types/matrix/matrix.hpp"
#include "types/matrix/ceres/ceres.hpp"


#define MOTOR_TYPE_STEPPER 0
#define MOTOR_TYPE_BLDC 1
#define MOTOR_TYPE MOTOR_TYPE_STEPPER

// #include "dsp/filter/EKF.hpp"
using namespace yumud::drivers;
using namespace yumud::foc;


struct TurnSolver{
    uint16_t ta = 0;
    uint16_t tb = 0;
    real_t pa = 0;
    real_t pb = 0;
    real_t va = 0;
    real_t vb = 0;
};

TurnSolver turnSolver;

real_t demo(uint milliseconds, uint microseconds = 0){
    using Vector2 = CubicInterpolation::Vector2;
    
    uint32_t turnCnt = milliseconds % 2667;
    uint32_t turns = milliseconds / 2667;
    
    scexpr real_t velPoints[7] = {
        real_t(20)/360, real_t(20)/360, real_t(62.4)/360, real_t(62.4)/360, real_t(20.0)/360, real_t(20.0)/360, real_t(20.0)/360
    };
    
    scexpr real_t posPoints[7] = {
        real_t(1.0f)/360,real_t(106.1f)/360,real_t(108.1f)/360, real_t(126.65f)/360, real_t(233.35f)/360,real_t(359.0f)/360,real_t(361.0f)/360
    };

    scexpr uint tickPoints[7] = {
        0, 300, 400, 500, 2210, 2567, 2667 
    };

    int8_t i = 6;

    while((turnCnt < tickPoints[i]) && (i > -1))
        i--;
    
    turnSolver.ta = tickPoints[i];
    turnSolver.tb = tickPoints[i + 1];
    auto dt = turnSolver.tb - turnSolver.ta;

    turnSolver.va = velPoints[i];
    turnSolver.vb = velPoints[i + 1];
    
    turnSolver.pa = posPoints[i];
    turnSolver.pb = posPoints[i + 1];
    real_t dp = turnSolver.pb - turnSolver.pa;

    real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / dt);
    real_t temp = (real_t)dt / 1000 / dp; 

    real_t yt = 0;

    if((i == 0) || (i == 2) || (i == 4))
        yt = CubicInterpolation::forward(Vector2{real_t(0.4f), real_t(0.4f) * turnSolver.va * temp}, Vector2(real_t(0.6f), real_t(1.0f) - real_t(0.4f)  * turnSolver.vb * temp), _t);
    else
        yt = _t;

    real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;

    return new_pos;
}


void stepper_tb(UartHw & logger){
    using TimerUtils::Mode;
    using TimerUtils::IT;

    logger.init(576000, CommMethod::Dma);
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

    timer1.oc(1).setPolarity(false);
    timer1.oc(2).setPolarity(false);
    timer1.oc(3).setPolarity(false);
    timer1.oc(4).setPolarity(false);
    
    using AdcChannelEnum = AdcUtils::Channel;
    using AdcCycleEnum = AdcUtils::SampleCycles;

    auto get_default_id = []() -> uint8_t {
        auto chip_id = Sys::Chip::getChipIdCrc();
        switch(chip_id){
            case 3273134334:
                return 3;
            case 341554774:
                return 2;
            case 4079188777:
                return 1;
            case 0x551C4DEA:
                return  3;
            case 0x8E268D66:
                return 1;
            default:
                return 0;
        }
    };

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

    spi1.init(18_M);
    spi1.bindCsPin(portA[15], 0);

    MT6816 encoder{{spi1, 0}};
    // MT6701 encoder{{spi1, 0}};

    I2cSw i2cSw{portD[1], portD[0]};
    i2cSw.init(400_K);
    AT24C02 at24{i2cSw};
    Memory mem{at24};


    FOCStepper stp{get_default_id(), svpwm, encoder, mem};
    FOCMotor::AsciiProtocol ascii_p{logger, stp};
    FOCMotor::CanProtocol can_p{can1, stp};

    stp.bindProtocol(ascii_p);
    stp.bindProtocol(can_p);

    timer3.init(foc_freq, Mode::CenterAlignedDownTrig);
    timer3.enableArrSync();
    timer3.bindCb(IT::Update, [&](){
        static int cnt = 0;
        cnt++;
        // if(cnt == chopper_freq / foc_freq){
        stp.tick();
            // cnt = 0;
        // }
    });
    timer3.enableIt(IT::Update,{0,0});

    can1.init(Can::BaudRate::_1M);
 
    stp.init();
    stp.setSpeedLimit(80);
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

        // Matrix_t<real_t, 2, 2> a;
        // a.at(0, 0) = 1;
        // a.at(0, 1) = 0;

        
        // Matrix_t<real_t, 2, 2> b;
        // b.at(1, 0) = 0;
        // b.at(1, 1) = 1;

        // Jet_t<real_t, 3> jet = {1};

        // DEBUG_PRINTLN(std::setprecision(2), a, b, a + b, (a + b).inverse());

        // auto f = [](const real_t x){return (x > 0) ? (x > real_t(0.2)) ? real_t(0.2) * x - real_t(0.04) : x * x : 0;};
        // real_t target = f(t-2);
        // real_t target = real_t(0.02) * sin(3 * t);
        // real_t target = real_t(0.02) * t;
        // real_t target = real_t(40.01) * CLAMP2((sin(t) * sign(fmod(t, real_t(PI)) - real_t(PI/2))), real_t(0.5));
        // real_t target = 
        // real_t target = real_t(1.01) * sin(16 *t);
        // target = 
        // static real_t target_filtered = 0;
        // target_filtered = ((target >> 8) * 1 + (target_filtered >> 8) *8191) >> 5;
        // stp.setTargetPosition(target);
        // stp.setTargetVector(target);
        // stp.setTargetSpeed(5);
        // stp.setTargetVector(target);

        // if(logger.pending() == 0) logger.println(stp.getTarget(), stp.getPosition(), stp.getSpeed(), stp.getCurrent(), real_t(adc1.inj(1)), real_t(adc1.inj(2)));
        auto target = demo(millis());
        if(logger.pending() == 0) logger.println(target, stp.getPosition(), stp.getSpeed(), stp.getCurrent(), stp.getRaddiff());
        // Sys::Clock::reCalculateTime();

        // stp.setTargetPosition(5 * sin(7 * t));
        // stp.setTargetPosition(target);
        // stp.setTargetSpeed(1);

        // stp.setTargetPosition(17* sin(2 * t));
        // stp.setTargetPosition(7 * frac(t));

        // real_t target = real_t(0.7) * sin(t)
        // stp.setTargetPosition(target);
        // stp.setTargetCurrent(target);
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN(stp.getPosition(), target);

        // real_t target = real_t(20.4) * sin(7 * t);
        // real_t target = real_t(30.4) * sign(sin(5 * t));
        // real_t target = real_t(0.8) * sign(sin(5 * t));
        // stp.setTargetSpeed(target);
        // stp.setTargetCurrent(target);
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN(stp.getPosition(), stp.getTarget(), stp.getSpeed(), stp.getCurrent());
        
        // stp.setTargetPosition(0);
        // stp.setTargetPosition(abs(frac(t)-real_t(0.5)));
        // stp.setTargetPosition(CLAMP(sin(3 * t), real_t(-0.5), real_t(0.5)));
        // Sys::Clock::reCalculateTime();
        // stp.setTargetPosition(4 * sin(t * 2) + sign(sin(t * 2)));
        // stp.setTargetPosition(3 * sin(10*t)*(cos(t/2)));
        // stp.setTargetPosition(5 *sin(t) * sin(t*5));
        // stp.setTargetPosition(frac(t));
        // stp.setTargetPosition(24 * sin(2 * t));
        // stp.setTargetPosition(2 * sin(24 * t));
        // stp.setTargetPosition(10 * int(7 * 6sin(t / 2)));
        // stp.setTargetPosition(3 * abs(frac(t*2) - real_t(0.5)));

        // stp.setTargetPosition(round(stp.getPosition() * 100)/100);
        // stp.setTargetPosition(3 * (sin(t * 2)));
        // stp.setTargetPosition(real_t(1.5) * sin(3 * t));
        // stp.setTargetSpeed(CLAMP(60 * sin(t * 3), 0, 30));
        // stp.setTargetTrapezoid(10 * sign(sin(3* t)));
        // stp.setTargetTrapezoid(10 * sign(sin(1.5 * t)));
        // stp.setTargetPosition(10 * sign(sin(1.5 * t)));
        // stp.setTargetPosition(40 * sin(t/2) + 20 * sign(sin(t/2)));
        // stp.setTargetPosition(real_t(0.3) * sin(t));
        // stp.setTargetPosition(sin(t) / 4);
        // stp.setTargetPosition(4 * sign(sin(4 * t)));
        Sys::Clock::reCalculateTime();
    }
}
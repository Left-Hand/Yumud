#include "src/testbench/tb.h"

#if 0

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/can/can.hpp"

#include "robots/foc/stepper/stepper.hpp"
#include "algo/interpolation/cubic.hpp"

#include "drivers/Storage/EEprom/AT24CXX/at24cxx.hpp"
#include "drivers/Encoder/MagEncoder.hpp"
#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/Encoder/MagEnc/MT6701/mt6701.hpp" 
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"

#include "drivers/GateDriver/MP6540/mp6540.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#define MOTOR_TYPE_STEPPER 0
#define MOTOR_TYPE_BLDC 1
#define MOTOR_TYPE MOTOR_TYPE_STEPPER

// #include "dsp/filter/EKF.hpp"

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;
using namespace ymd::intp;


uint8_t get_default_id(){

    auto node_id = [](){
        auto chip_id = sys::chip::get_chip_id_crc();
        // DEBUG_PRINTLN("myididididdid", chip_id)
        switch(chip_id){
            case 3273134334:
                return 3;
            case 0xF3237B29:
                return 5;
            case 0x145BB656:
                return 1;
            case 0x551C4DEA:
                return  3;
            case 0x8E268D66:
                return 1;
            case 0xC803C4C6:
                return 2;
            default:
                return 0;
        }
    }();

    DEBUG_PRINTLN("node_id", node_id);
    return node_id;
};


void stepper_tb(UartHw & logger_inst){
    logger_inst.init(576000, CommStrategy::Dma);
    DEBUGGER.retarget(&logger_inst);
    DEBUGGER.set_eps(4);

    #if(MOTOR_TYPE == MOTOR_TYPE_STEPPER)

    auto & ena_gpio = hal::PB<0>();
    auto & enb_gpio = hal::PA<7>();

    AT8222 coilA{timer1.oc<3>(), timer1.oc<4>(), ena_gpio.outpp(HIGH)};
    AT8222 coilB{timer1.oc<1>(), timer1.oc<2>(), enb_gpio.outpp(HIGH)};

    coilA.init();
    coilB.init();

    digipw::SVPWM2 svpwm{coilA, coilB};

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
    
    svpwm.inverse(DISEN);
    
    timer1.init(chopper_freq, TimerCountMode::CenterAlignedDownTrig);
    timer1.enable_arr_sync();
    timer1.oc<1>().init({.valid_level = LOW});
    timer1.oc<2>().init({.valid_level = LOW});
    timer1.oc<3>().init({.valid_level = LOW});
    timer1.oc<4>().init({.valid_level = LOW});
    
    // using AdcChannelSelection = AdcChannelSelection;
    // using AdcCycleEnum = AdcSampleCycles;



    // adc1.init(
    //     {
    //         AdcChannelConfig{AdcChannelSelection::VREF, AdcCycleEnum::T28_5}
    //     },{
    //         AdcChannelConfig{AdcChannelSelection::CH4, AdcCycleEnum::T7_5},
    //         AdcChannelConfig{AdcChannelSelection::CH3, AdcCycleEnum::T7_5},
    //     }
    // );

    // timer1.setTrgoSource(TimerTrgoSource::Update);
    // timer1.setRepeatTimes(1);
    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1TRGO);
    // adc1.enableAutoInject(false);


    svpwm.enable();

    spi1.init(18_MHz);
    

    MT6816 encoder{{spi1, spi1.allocate_cs_pin(hal::PA<15>()).value()}};
    // MT6701 encoder{{spi1, 0}};

    I2cSw i2cSw{hal::PD<1>(), hal::PD<0>()};
    i2cSw.init(400_KHz);
    AT24CXX at24{AT24CXX::AT24C02{}, i2cSw};
    Memory mem{at24};

    uint8_t node_id = get_default_id();
    auto & can = can1;
    can.init(CanBaudrate::_1M);
    // can.enableHwReTransmit();
    
    can[0].mask(
        CanID16{uint16_t(uint16_t(node_id) << 7), CanRtr::Any},
        CanID16::IGNORE_LOW(7, CanRtr::Any),
        
        CanID16{0x000, CanRtr::Any}, 
        CanID16::IGNORE_LOW(7, CanRtr::Any));

    FOCStepper stp{node_id, svpwm, encoder, mem};
    FOCMotor::AsciiProtocol ascii_p{logger_inst, stp};
    FOCMotor::CanProtocol can_p{can, stp};

    stp.bindProtocol(ascii_p);
    stp.bindProtocol(can_p);

    timer3.init(foc_freq, TimerCountMode::CenterAlignedDownTrig);
    timer3.enable_arr_sync();
    timer3.bind_cb(TimerIT::Update, [&](){
        stp.tick();
    });

    timer3.enable_it(TimerIT::Update,{0,0});

 
    stp.init();
    stp.setSpeedLimit(80);
    stp.setAccLimit(172);
    stp.setOpenLoopCurrent(real_t(0.7));
    stp.setCurrentLimit(real_t(0.4));

    while(!stp.isActive());
    stp.setTargetCurrent(real_t(0));
    stp.setCurrentLimit(real_t(1.8));
    
    while(true){
        stp.run();
        stp.report();

        // Matrix<real_t, 2, 2> a;
        // a.at(0, 0) = 1;
        // a.at(0, 1) = 0;

        
        // Matrix<real_t, 2, 2> b;
        // b.at(1, 0) = 0;
        // b.at(1, 1) = 1;

        // Jet<real_t, 3> jet = {1};

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
        // auto target = demo(millis());
        // auto target = -floor(t>>1)<<1;
        // auto target = -demo2(t);
        // auto target = t * 0.2_r;
        // if(logger.pending() == 0) logger.println(target, stp.getPosition(), stp.getSpeed(), stp.getTargetEstSpeed(), stp.getCurrent(), stp.getRaddiff());
        // Sys::Clock::reCalculateTime();

        // stp.setTargetPosition(5 * sin(7 * t));

        // static uint last_millis = 0;
        // if(millis() - last_millis >= 5){
        //     stp.setTargetPosition(target);
        //     last_millis = millis();
        // }
        // stp.setTargetSpeed(1);

        // stp.setTargetPosition(17* sin(2 * t));
        // stp.setTargetPosition(7 * frac(t));

        // real_t target = real_t(0.7) * sin(t);
        // stp.setTargetPosition(target);
        // stp.setTargetVector(target);
        // stp.setTargetCurrent(target);
        // DEBUG_PRINTLN_IDLE(stp.getPosition(), target);

        // real_t target = real_t(20.4) * sin(7 * t);
        // real_t target = real_t(30.4) * sign(sin(5 * t));
        // real_t target = real_t(0.8) * sign(sin(5 * t));
        // stp.setTargetSpeed(target);
        // stp.setTargetCurrent(target);
        // DEBUG_PRINTLN_IDLE(stp.getPosition(), stp.getTarget(), stp.getSpeed(), stp.getCurrent());
        
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
        // Sys::Clock::reCalculateTimeMs();
    }
}

#endif
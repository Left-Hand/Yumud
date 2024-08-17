#include "tb.h"




void stepper_tb(IOStream & logger){
    using TimerUtils::Mode;
    using TimerUtils::IT;
    logger.setEps(4);

    auto & ena_gpio = TIM3_CH3_GPIO;
    auto & enb_gpio = TIM3_CH2_GPIO;
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

    Stepper stp{logger, can1, svpwm, mt6816, mem};

    timer3.init(foc_freq, Mode::CenterAlignedDownTrig);
    timer3.enableArrSync();
    timer3.bindCb(IT::Update, [&](){stp.tick();});
    timer3.enableIt(IT::Update, NvicPriority(0, 0));

    can1.init(Can::BaudRate::Mbps1);
 
    stp.init();
    // logger.println("stat is", stp.status()._to_string());
    // while(+stp.status() == +Stepper::RunStatus::ACTIVE);

    stp.setOpenLoopCurrent(0.8);
    stp.setCurrentClamp(1.4);
    stp.setTargetCurrent(-0.9);

    while(stp.isActive() == false);
    
    delay(3400);

    stp.locateRelatively(-3);
    stp.setCurrentClamp(1.4);

    while(true){
        stp.run(); 
        // stp.setTargetVector(sin(t));
        // stp.setTargetVector(2 * t);
        // logger.println(stp.getSpeed());
        stp.report();
        Sys::Clock::reCalculateTime();

        // stp.setTargetPosition(0.05 * t);
        // stp.setTargetCurrent(1.2 * sin(t));

        // stp.setTargetPosition(2.6 * sin(4 * t));
        // stp.setTargetPosition(20 * sign(sin(t)));
        // stp.setTargetTrapezoid(70 * floor(t / 3));

        // stp.setTargetPosition(0.2 * floor(t*10));
        // stp.setTargetPosition(sin(t) + sign(sin(t)) + 4);
        // stp.setTargetPosition(sin(t));
        stp.setTargetPosition(2.5 * sin(3 * t));
        // stp.setTargetPosition(-t/8);
        // stp.setTargetPosition(4 * floor(fmod(t * 4,2)));
        // real_t temp = sin(2 * t);
        // temp += 10 * sign(temp);
        // stp.setTargetSpeed(20 + temp);
        // stp.setTargetSpeed(20 * sin(t));
        // stp.setTargetSpeed(5);
        // stp.setTargetSpeed(sin(t));
        // stp.setTargetPosition(ss());
        // stp.setTargetSpeed(CLAMP(60 * sin(2 * t), 10, 35));
        // stp.setTargetSpeed((5 << (int(2 * t) % 4)));
        // stp.setTargetSpeed(5 * sin(2*t));
        // switch(int(t)){

        // real_t _t = fmod(t * 3, 6);
        // // stp.setTargetSpeed(CLAMP(40 * sin(PI / 2 * t), -20, 20));
        // if(IN_RANGE(_t, 1, 3))stp.setTargetSpeed(40);
        // else if(IN_RANGE(_t, 4, 6)) stp.setTargetSpeed(0);
        // else{
        //     real_t temp = sin(PI / 2 * _t);
        //     stp.setTargetSpeed(40 * temp * temp);
        // }
        // }

    }
}
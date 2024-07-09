#include "node.hpp"

void node_main(){
    using TimerUtils::Mode;
    using TimerUtils::IT;

    uart1.init(921600);
    auto & logger = uart1;
    logger.setEps(4);

    auto & ena_gpio = TIM3_CH3_Gpio;
    auto & enb_gpio = TIM3_CH2_Gpio;
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

    stp.setOpenLoopCurrent(0.8);
    stp.setCurrentClamp(1.4);

    logger.println(stp.getNodeId());
    while(true){
        stp.run(); 
        Sys::Clock::reCalculateTime();
    }
}
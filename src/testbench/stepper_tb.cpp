#include "tb.h"

struct TurnSolver{
    uint16_t ta = 0.0f;
    uint16_t tb = 0.0f;
    real_t pa = 0.0f;
    real_t pb = 0.0f;
    real_t va = 0.0f;
    real_t vb = 0.0f;
}turnSolver;
// Helper functions:
real_t slopeFromT (real_t t, real_t A, real_t B, real_t C){
  real_t dtdx = 1.0/(3.0*A*t*t + 2.0*B*t + C); 
  return dtdx;
}

real_t xFromT (real_t t, real_t A, real_t B, real_t C, real_t D){
  real_t x = A*(t*t*t) + B*(t*t) + C*t + D;
  return x;
}

real_t yFromT (real_t t, real_t E, real_t F, real_t G, real_t H){
  real_t y = E*(t*t*t) + F*(t*t) + G*t + H;
  return y;
}

real_t liner(real_t t, real_t A, real_t B){
    return t * B + (1.0f - t) * A;
}
real_t cubicBezier(real_t x, const Vector2 & a,const Vector2 & b){

  real_t y0a = 0.0; // initial y
  real_t x0a = 0.0; // initial x 
  real_t y1a = a.y;    // 1st influence y   
  real_t x1a = a.x;    // 1st influence x 
  real_t y2a = b.y;    // 2nd influence y
  real_t x2a = b.x;    // 2nd influence x
  real_t y3a = 1.0; // final y 
  real_t x3a = 1.0; // final x 

  real_t A =   x3a - 3.0*x2a + 3.0*x1a - x0a;
  real_t B = 3.0*x2a - 6.0*x1a + 3.0*x0a;
  real_t C = 3.0*x1a - 3.0*x0a;   
  real_t D =   x0a;

  real_t E =   y3a - 3.0*y2a + 3.0*y1a - y0a;    
  real_t F = 3.0*y2a - 6.0*y1a + 3.0*y0a;             
  real_t G = 3.0*y1a - 3.0*y0a;             
  real_t H =   y0a;

  real_t currentt = x;
  for (int i=0; i < 5; i++){
    real_t currentx = xFromT (currentt, A,B,C,D); 
    real_t currentslope = slopeFromT (currentt, A,B,C);
    currentt -= (currentx - x)*(currentslope);
  	currentt = CLAMP(currentt,real_t(0),real_t(1)); 
  } 

  real_t y = yFromT (currentt,  E,F,G,H);
  return y;
}



real_t ss(){
    auto turnCnt = millis() % 2667;
    uint32_t turns = millis() / 2667;
    
    real_t velPoints[7] = {
        20.0f/360, 20.0f/360, 62.4f/360, 62.4f/360, 20.0f/360, 20.0f/360, 20.0f/360
    };
    
    real_t posPoints[7] = {
        1.0f/360, 106.1f/360, 108.1f/360, 126.65f/360, 233.35f/360, 359.0f/360, 361.0f/360
    };

    uint16_t tickPoints[7] = {
        0, 300, 400, 500, 2210, 2567, 2667 
    };

    int8_t i = 6;

    while((turnCnt < tickPoints[i]) && (i > -1))
        i--;
    
    turnSolver.ta = tickPoints[i];
    turnSolver.tb = tickPoints[i + 1];
    uint16_t dt = turnSolver.tb - turnSolver.ta;

    turnSolver.va = velPoints[i];
    turnSolver.vb = velPoints[i + 1];
    
    turnSolver.pa = posPoints[i];
    turnSolver.pb = posPoints[i + 1];
    real_t dp = turnSolver.pb - turnSolver.pa;

    real_t t = ((real_t)(turnCnt  - turnSolver.ta) / (real_t)dt);
    real_t temp = (real_t)dt / 1000 / dp; 

    real_t yt = 0.0f;

    if((i == 0) || (i == 2) || (i == 4))
        yt = cubicBezier(t, Vector2{real_t(0.4f), 0.4f * turnSolver.va * temp}, Vector2(real_t(0.6f), 1.0f - 0.4f * turnSolver.vb * temp));
    else
        yt = t;

    real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;
    // turnSolver.pos = new_pos;

    // stp.setTargetPosition(new_pos);
    return new_pos;
    // stp.setTagretVector(new_pos*2);
}


void stepper_tb(IOStream & logger){
    using TimerUtils::Mode;
    using TimerUtils::IT;
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

    Stepper stp{logger, svpwm, mt6816, mem};

    timer3.init(foc_freq, Mode::CenterAlignedDownTrig);
    timer3.enableArrSync();
    timer3.bindCb(IT::Update, [&](){stp.tick();});
    timer3.enableIt(IT::Update, NvicPriority(0, 0));

    can1.init(Can::BaudRate::Mbps1);
 
    stp.init();

    stp.setCurrentClamp(1.4);
    stp.setOpenLoopCurrent(0.8);
    while(true){
        stp.run();
        // stp.setTagretVector(sin(t));
        // stp.setTagretVector(2 * t);
        
        stp.report();
        Sys::Clock::reCalculateTime();

        // stp.setTargetPosition(0.05 * t);
        // stp.setTargetCurrent(1.2 * sin(t));

        // stp.setTargetPosition(2.6 * sin(4 * t));
        // stp.setTargetPosition(20 * sign(sin(t)));
        // stp.setTagretTrapezoid(70 * floor(t / 3));

        // stp.setTargetPosition(0.2 * floor(t*10));
        // stp.setTargetPosition(sin(t) + sign(sin(t)) + 4);
        // stp.setTargetPosition(sin(t));
        stp.setTargetPosition(0.1 * sin(8 * t));
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
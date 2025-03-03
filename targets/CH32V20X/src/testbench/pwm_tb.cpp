#include "tb.h"

#include "../hal/timer/pwm/gpio_pwm.hpp"

#define PWM_TB_GPIO
// #define PWM_TB_TIMEROC
// #define PWM_TB_CO


void pwm_tb(OutputStream & logger){

    timer1.init(36000);
    #ifdef PWM_TB_GPIO
    GpioPwm pwm{portA[8], 32};

    timer1.enableIt(TimerIT::Update, {0,0});
    timer1.bindCb(TimerIT::Update, [&](){pwm.tick();});

    #endif

    #ifdef PWM_TB_TIMEROC
    PwmIntf & pwm = timer1.oc(1);
    

    #ifdef PWM_TB_CO
    auto & co = timer1.ocn(1);
    co.init();
    co.setPolarity(HIGH);
    co.setIdleState(HIGH);

    timer1.initBdtr();
    #endif
    #endif

    pwm.init();

    while(true){
        logger.println(t, pwm.cnt(), pwm.cvr());
        pwm = 0.5 * sin(4 * t) + 0.5;
        delayMicroseconds(100);
    }

}
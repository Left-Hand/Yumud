#include "tb.h"

#define PWM_TB_SIN
#define PWM_TB_CO


void pwm_tb(OutputStream & logger){
    timer1.init(36000);
    PwmChannel & pwm = timer1.oc(1);
    
    pwm.init();


    #ifdef PWM_TB_SIN

    #ifdef PWM_TB_CO
    // timer1.ocn(1).init();
    // timer1.ocn(1).setPolarity(false);

    timer1.initBdtr();
    #endif

    while(true){
        logger.println(t);
        pwm = 0.5 * sin(4 * t) + 0.5;
        delayMicroseconds(100);
    }
    #endif

}
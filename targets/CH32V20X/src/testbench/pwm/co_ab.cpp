#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"

#include "hal/timer/instance/timer_hw.hpp"

class TimerOCTwins:public PwmIntf{
protected:
    TimerOC & oc_;
    TimerOCN & ocn_;
    bool last_polar = false;
public:
    TimerOCTwins(AdvancedTimer & timer, const size_t idx):
        oc_(timer.oc(idx)),
        ocn_(timer.ocn(idx)){;}

    TimerOCTwins(TimerOC & oc, TimerOCN & ocn):
        oc_(oc), ocn_(ocn){;}

    TimerOCTwins & operator = (const real_t value) override{
        const bool polar = value > 0;
        if(last_polar != polar){
            last_polar = polar;

            if(polar){
                oc_.setOutputState(true);
                ocn_.setOutputState(false);
            }else{
                oc_.setOutputState(false);
                ocn_.setOutputState(true);
            }
        }
        oc_ = abs(value);
        return *this;
    }
};

void co_ab_main(){
    uart2.init(DEBUG_UART_BAUD);
    DEBUGGER.retarget(&uart2);

    auto & timer = timer1;
    auto & pwm_p = timer.oc(1);
    auto & pwm_n = timer.ocn(1);

    timer.init(10_KHz);

    pwm_p.init();
    pwm_n.init();

    pwm_p.setIdleState(true);
    pwm_n.setIdleState(true);
    TimerOCTwins pwm_pair{pwm_p, pwm_n};

    while(true){
        DEBUG_PRINTLN(millis());
        // pwm_gpio.toggle();
        // auto prog = sin(10 * t) * 0.5_r + 0.5_r;
        auto prog = sin(6 * time());
        // pwm_gpio = bool(prog > 0.5_r);
        // pwm_p = prog;
        pwm_pair = prog;
        // delay(200);
    }    
}
#pragma once

#include "GateDriverIntf.hpp"

namespace ymd::drivers{

class TB67H450{
protected:
    TimerOC & forward_pwm;
    TimerOC & backward_pwm;
    PwmIntf & vref_pwm;
    bool enabled = true;
    bool softmode = true;
    real_t inv_fullscale = (1);
public:
    TB67H450(TimerOC & _forward_pwm, TimerOC & _backward_pwm, PwmIntf & _vref_pwm):
            forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), vref_pwm(_vref_pwm){;}

    void init(){
        forward_pwm.setPolarity(false);
        backward_pwm.setPolarity(false);

        forward_pwm.init();
        backward_pwm.init();
        vref_pwm.init();

        setClamp(1.0);
    }

    void set_clamp(const real_t abs_max_value){
        vref_pwm = ABS(abs_max_value);
    }

    void enable(const Enable en = EN){
        enabled = en;
        if(!en){
            forward_pwm = real_t(1);
            backward_pwm = real_t(1);
            vref_pwm = real_t(0);
        }
    }

    void set_current(const real_t curr){
        if(curr > 0){
            forward_pwm = 0;
            backward_pwm = curr * inv_fullscale;
        }else if(curr < 0){
            forward_pwm = -curr * inv_fullscale;
            backward_pwm = 0;
        }else{
            forward_pwm = 0;
            backward_pwm = 0;
        }
    
    }

    TB67H450 & operator = (const real_t curr){set_current(curr); return *this;}
};


};
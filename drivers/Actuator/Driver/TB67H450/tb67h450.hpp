#pragma once

#include "CoilDriver.hpp"

namespace yumud::drivers{

class TB67H450:public Coil2Driver{
protected:
    TimerOC & forward_pwm;
    TimerOC & backward_pwm;
    PwmChannel & vref_pwm;
    bool enabled = true;
    bool softmode = true;
    real_t inv_fullscale = (1);
public:
    TB67H450(TimerOC & _forward_pwm, TimerOC & _backward_pwm, PwmChannel & _vref_pwm):
            forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), vref_pwm(_vref_pwm){;}

    void init(){
        forward_pwm.setPolarity(false);
        backward_pwm.setPolarity(false);

        forward_pwm.init();
        backward_pwm.init();
        vref_pwm.init();

        setClamp(1.0);
    }

    void setClamp(const real_t abs_max_value){
        vref_pwm = ABS(abs_max_value);
    }

    void enable(const bool en = true){
        enabled = en;
        if(!en){
            forward_pwm = real_t(1);
            backward_pwm = real_t(1);
            vref_pwm = real_t(0);
        }
    }

    void setCurrent(const real_t curr){
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

    TB67H450 & operator = (const real_t curr){setCurrent(curr); return *this;}
};
class Coil2:public Coil2Driver{
protected:
    PwmChannel & instanceP;
    PwmChannel & instanceN;
    bool enabled = true;
public:
    Coil2(PwmChannel & _instanceP, PwmChannel & _instanceN):instanceP(_instanceP), instanceN(_instanceN){;}

    void init() override{
        instanceP.init();
        instanceN.init();
    }

    // void setClamp(const real_t max_value) override{
    //     real_t abs_max_value = abs(max_value);
    //     instanceP.setClamp(abs_max_value);
    //     instanceN.setClamp(abs_max_value);
    // }

    // void enable(const bool en = true) override{
    //     enabled = en;
    //     if(!en) setDuty(real_t(0));
    // }

    // void setDuty(const real_t duty) override{
    //     if(!enabled){
    //         instanceP = 0;
    //         instanceN = 0;
    //         return;
    //     }
    //     if(duty > 0){
    //         instanceP = duty;
    //         instanceN = 0;
    //     }else{
    //         instanceP = 0;
    //         instanceN = -duty;
    //     }
    // }

    // Coil2 & operator = (const real_t duty) override {setDuty(duty); return *this;}
};

};
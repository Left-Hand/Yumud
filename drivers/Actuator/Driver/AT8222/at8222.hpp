#pragma once

#include "../CoilDriver.hpp"

class AT8222:public Coil2Driver{
protected:
    TimerOC & forward_pwm;
    TimerOC & backward_pwm;
    GpioConcept * p_enable_gpio;
    real_t inv_fullscale = real_t(0.5);

    AT8222(TimerOC & _forward_pwm, TimerOC & _backward_pwm, GpioConcept * _p_en_gpio = nullptr):
        forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), p_enable_gpio(_p_en_gpio){;}
public:

    AT8222(TimerOC & _forward_pwm, TimerOC & _backward_pwm, GpioConcept & _en_gpio):
            AT8222(_forward_pwm, _backward_pwm, &_en_gpio){;}

    AT8222(TimerOC & _forward_pwm, TimerOC & _backward_pwm):
            AT8222(_forward_pwm, _backward_pwm, nullptr){;}


    void init(){
        forward_pwm.setPolarity(false);
        backward_pwm.setPolarity(false);

        if(p_enable_gpio) p_enable_gpio->set();
    }

    void enable(const bool en = true){
        if(p_enable_gpio) p_enable_gpio->write(en);
        if(!en){
            forward_pwm = real_t(1);
            backward_pwm = real_t(1);
        }
    }

    AT8222 & operator = (const real_t duty){
        if(duty > 0){
            forward_pwm = 0;
            backward_pwm = duty * inv_fullscale;
        }else if(duty < 0){
            forward_pwm = -duty * inv_fullscale;
            backward_pwm = 0;
        }else{
            forward_pwm = 0;
            backward_pwm = 0;
        }
     return *this;}
};
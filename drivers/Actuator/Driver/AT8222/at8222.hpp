#pragma once

#include "../CoilDriver.hpp"

class AT8222:public Coil2Driver{
protected:
    TimerOC & forward_pwm;
    TimerOC & backward_pwm;
    GpioConcept * p_enable_gpio;
    static constexpr real_t inv_fullscale = real_t(0.5);

    AT8222(TimerOC & _forward_pwm, TimerOC & _backward_pwm, GpioConcept * _p_en_gpio = nullptr):
        forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), p_enable_gpio(_p_en_gpio){;}
public:

    AT8222(TimerOC & _forward_pwm, TimerOC & _backward_pwm, GpioConcept & _en_gpio):
            AT8222(_forward_pwm, _backward_pwm, &_en_gpio){;}

    AT8222(TimerOC & _forward_pwm, TimerOC & _backward_pwm):
            AT8222(_forward_pwm, _backward_pwm, nullptr){;}


    void init();
    void enable(const bool en = true);

    AT8222 & operator = (const real_t duty);
};
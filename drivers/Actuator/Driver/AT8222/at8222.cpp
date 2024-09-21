#include "AT8222.hpp"

void AT8222::init(){
    forward_pwm.setPolarity(false);
    backward_pwm.setPolarity(false);

    if(p_enable_gpio) p_enable_gpio->set();
}

void AT8222::enable(const bool en){
    if(p_enable_gpio) p_enable_gpio->write(en);
    if(!en){
        forward_pwm = real_t(1);
        backward_pwm = real_t(1);
    }
}

AT8222 & AT8222::operator = (const real_t duty){
    if(duty > 0){
        forward_pwm = 0;
        backward_pwm = duty;
    }else if(duty < 0){
        forward_pwm = -duty;
        backward_pwm = 0;
    }else{
        forward_pwm = 0;
        backward_pwm = 0;
    }
    return *this;}
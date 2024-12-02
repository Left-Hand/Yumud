#include "nozzle.hpp"

namespace gxm{
    
void Nozzle::release(){
    // gpio_a_.clr();
    // gpio_b_.set(); 
    pwm_ = 0;
}

void Nozzle::press(){
    // gpio_a_.set();
    // gpio_b_.clr(); 
    pwm_ = real_t(0.99);
}

void Nozzle::off(){
    // gpio_a_.clr();
    // gpio_b_.clr();   
    pwm_ = 0;
}


}
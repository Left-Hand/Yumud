#include "nozzle.hpp"

namespace gxm{
    
void Nozzle::release(){
    gpio_a_.clr();
    gpio_b_.set(); 
}

void Nozzle::press(){
    gpio_a_.set();
    gpio_b_.clr(); 
}

void Nozzle::off(){
    gpio_a_.clr();
    gpio_b_.clr();   
}


}
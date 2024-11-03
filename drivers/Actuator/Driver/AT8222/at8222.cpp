#include "AT8222.hpp"


using namespace yumud;
using namespace yumud::drivers;
    
    
void AT8222::init(){
    enable();
}

void AT8222::enable(const bool en){
    if(p_enable_gpio) p_enable_gpio->write(en);
    if(!en){
        forward_pwm = real_t(1);
        backward_pwm = real_t(1);
    }
}


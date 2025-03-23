#include "AT8222.hpp"


using namespace ymd;
using namespace ymd::drivers;
    
    
void AT8222::init(){
    enable();
}

void AT8222::enable(const bool en){
    enable_gpio_.write(en);
    if(!en){
        forward_pwm_ = real_t(1);
        backward_pwm_ = real_t(1);
    }
}


#include "AT8222.hpp"


using namespace ymd;
using namespace ymd::drivers;
    
    
void AT8222::init(){
    enable();
}

void AT8222::enable(const bool en){
    enable_gpio_.write(BoolLevel::from(en));
    if(!en){
        forward_pwm_.set_duty(real_t(1));
        backward_pwm_.set_duty(real_t(1));
    }
}


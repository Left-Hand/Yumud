#include "tb.h"

#include "drivers/Actuator/DJI/M3508/m3508.hpp"
#include "hal/timer/instance/timer_hw.hpp"

void m3508_main(){
    scexpr real_t max_curr = real_t(1.8);
    scexpr size_t cb_freq = 50;

    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);
    can1.init(1_M);

    M3508Port port{can1};
    
    timer3.init(cb_freq);
    timer3.bindCb(TimerUtils::IT::Update, [&](){
        port.tick();
    });
    
    while(true){
        port[1].setTargetCurrent(max_curr * sin(t));   
        port[2].setTargetCurrent(max_curr * cos(t));   
        port[3].setTargetCurrent(max_curr * -sin(t));   
        port[4].setTargetCurrent(max_curr * -cos(t));

        DEBUGGER.println(port[1].getCurrent(), port[2].getCurrent(), port[3].getCurrent(), port[4].getCurrent()); 
    }
}
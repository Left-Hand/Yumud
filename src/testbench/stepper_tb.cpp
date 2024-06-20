#include "tb.h"

void stepper_tb(IOStream & logger){
    Stepper stp;
    can1.init(Can::BaudRate::Mbps1);
    stp.init();
    stp.setCurrentClamp(0.1);
    while(true){
        stp.run();
        stp.setTargetPosition(sin(t));
        // stp.setTargetSpeed(16 * sin(t));
    }
}
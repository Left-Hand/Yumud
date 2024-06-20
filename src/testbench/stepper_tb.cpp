#include "tb.h"

void stepper_tb(IOStream & logger){
    Stepper stp;
    can1.init(Can::BaudRate::Mbps1);
    stp.init();
    stp.setCurrentClamp(0.3);
    while(true){
        stp.run();
        // stp.setTargetPosition(0.005 + 0.002 * sin(t));
        // stp.setTargetPosition(7 * sin(t));
        stp.setTargetPosition(16 * sin(t));
        // stp.setTargetSpeed(16 * sin(t));
    }
}
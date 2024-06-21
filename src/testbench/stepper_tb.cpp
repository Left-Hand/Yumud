#include "tb.h"

void stepper_tb(IOStream & logger){
    Stepper stp;
    can1.init(Can::BaudRate::Mbps1);
    stp.init();
    stp.setCurrentClamp(1.2);
    while(true){
        stp.run();
        // stp.setTargetPosition(0.005 + 0.002 * sin(t));
        // stp.setTargetPosition(1 * sin(7 * t));
        // stp.setTargetPosition(0.8 * floor(6 * t));
        stp.setTargetPosition(20 * sin(t));
        // stp.setTargetSpeed(16 * sin(t));
    }
}
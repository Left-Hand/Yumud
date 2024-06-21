#include "tb.h"

void stepper_tb(IOStream & logger){
    Stepper stp;
    can1.init(Can::BaudRate::Mbps1);
    stp.init();
    stp.setCurrentClamp(1.2);
    while(true){
        stp.run();
        // stp.setTargetPosition(0.005 + 0.002 * sin(t));
        // stp.setTargetPosition(1 * sin(t));
        // stp.setTargetPosition(0.8 * floor(6 * t));
        // stp.setTargetPosition(20 * sin(t));
        // stp.setTargetPosition(2.4 * sin(t));
        // stp.setTargetSpeed(2 + sin(t));
        stp.setTargetSpeed((2 << (int(1.5 * t) % 3)));
    }
}
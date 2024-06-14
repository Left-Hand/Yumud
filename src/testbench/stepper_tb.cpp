#include "tb.h"

void stepper_tb(IOStream & logger){
    Stepper stp;
    uart1.init(115200 * 8);
    can1.init(Can::BaudRate::Mbps1);

    stp.init();
    stp.setCurrentClamp(0.3);
    while(true){
        stp.run();
        stp.setTargetPosition(8*sin(t));

        stp.setTargetSpeed(30 + 5 * sin(t));
    }
}
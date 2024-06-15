#include "tb.h"

void stepper_tb(OutputStream & logger){
    Stepper stp;
    uart1.init(115200 * 8);
    can1.init(Can::BaudRate::Mbps1);
    // logger.println("???");
    stp.init();
    stp.setCurrentClamp(0.2);
    while(true){
        stp.run();
        stp.setTargetPosition(16*sin(t));

        // stp.setTargetSpeed(16 * sin(t));
    }
}
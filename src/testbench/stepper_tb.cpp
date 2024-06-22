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
        stp.setTargetPosition(70 * floor(t/4));
        // stp.setTargetPosition(30 * sin(t));
        // stp.setTargetPosition(2.4 * sin(t));
        // stp.setTargetSpeed(2 + sin(t));
        // stp.setTargetSpeed(-(2 << (int(1.5 * t) % 3)));
        // switch(int(t)){

        // real_t _t = fmod(t * 3, 6);
        // // stp.setTargetSpeed(CLAMP(40 * sin(PI / 2 * t), -20, 20));
        // if(IN_RANGE(_t, 1, 3))stp.setTargetSpeed(40);
        // else if(IN_RANGE(_t, 4, 6)) stp.setTargetSpeed(0);
        // else{
        //     real_t temp = sin(PI / 2 * _t);
        //     stp.setTargetSpeed(40 * temp * temp);
        // }
        // }

    }
}
#include "tb.h"

void stepper_tb(IOStream & logger){
    Stepper stp;
    // can1.init(Can::BaudRate::Mbps1);
    stp.init();
    stp.setCurrentClamp(1.2);
    while(true){
        stp.run();
        // stp.setTagretVector(2 * sin(t));
        // stp.setTagretVector(2 * t);
        
        stp.report();
        Sys::Clock::reCalculateTime();

        stp.setTargetPosition(0.005 + 0.02 * sin(8 * t));
        // stp.setTargetPosition(2 * sin(8 * t));
        // stp.setTargetPosition(20 * floor(t/3));
        // stp.setTagretTrapezoid(70 * floor(t / 3));

        // stp.setTargetPosition(0.2 * floor(t*32));
        // stp.setTargetPosition(sin(t) + sign(sin(t)) + 4);
        // stp.setTargetPosition(16 * sin(t));
        // stp.setTargetPosition(t/8);
        // stp.setTargetPosition(-t/8);
        // stp.setTargetPosition(4 * floor(fmod(t * 4,2)));
        // real_t temp = sin(2 * t);
        // temp += 10 * sign(temp);
        // stp.setTargetSpeed(20 + temp);
        // stp.setTargetSpeed(sin(t));
        // stp.setTargetSpeed(5);
        // stp.setTargetSpeed(2 * sin(6*t));
        // stp.setTargetSpeed(CLAMP(60 * sin(2 * t), 10, 35));
        // stp.setTargetSpeed((5 << (int(2 * t) % 4)));
        // stp.setTargetSpeed(20);
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
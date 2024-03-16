#include "apps.h"


void chassis_app(){
    uart2.init(115200*4);
    Printer & log = uart2;
    log.setEps(4);
    log.println("HI");

    auto TrigA = Gpio(GPIOA, Pin::_0);
    TrigA.InPullUP();
    auto TrigB = Gpio(GPIOA, Pin::_1);
    TrigB.InPullUP();

    auto ch1 = ExtiChannel(TrigA, 1, 2, ExtiChannel::Trigger::RisingFalling);
    auto cap1 = CaptureChannelExti(ch1, true);
    cap1.init();
    // auto ch2 = ExtiChannel(TrigB, 1, 2);
    // ch2.bindCb([&log](){log.println("2");});
    // ch2.init();

    while(1){
        log.println(cap1.getFreq(), cap1.getDuty());
        delay(100);
    }
};
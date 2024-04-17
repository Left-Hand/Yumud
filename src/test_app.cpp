#ifndef __TEST_APP__

#define __TEST_APP__

#include <vector>
#include "apps.h"
#include "fwwb_compents/fwwb_inc.h"
#include "fwwb_compents/stations/access_point.hpp"


using namespace FWWB;
using namespace Sys::Clock;
using namespace Sys::Chip;

static Printer & logger = uart2;



void test_app(){
    // bool tx_role = Sys::getChipId() == 6002379527825632205;
    uart2.init(115200);
    logger.setRadix(16);
    logger.setSpace("");
    logger.println("ChipId: ", getChipId());
    logger.println("ChipId: ", getChipIdCrc());
    logger.setRadix(10);
    logger.setSpace(",");

    Gpio bled(GPIOC, Pin::_13);
    bled.OutPP();
    CanAcessPoint ap(can1, logger);
    delay(100);
    ap.init();
    // volatile uint32_t last_blink_millis = 0;
    while(true){
        // if((millis() > last_blink_millis) and (millis() % 200 == 0)){
        //     // logger.println("run");
        //     // bled= !bled;
        //     last_blink_millis = millis();
        // }
        // logger.println("run");
        ap.run();
        // delay(1);
        // can1.write(CanMsg((uint8_t)Command::ACTIVE << 4, true));
        // delay(1);
        // can1.write(CanMsg((uint8_t)Command::RST << 4, true));
        // // ap.parseCommand()
        // // delay(20);
        // ap.parseLine("O 1");
        // delay(200);
        // ap.parseLine("O -1");
        // delay(200);
        reCalculateTime();
    }
    // can1.init(Can1::BaudRate::Mbps1);
    // uart2.println("can test");
    // while(1){
    //     if(tx_role){
    //         static uint8_t cnt = 0;
    //         // CanMsg msg_v = CanMsg(cnt << 4, {cnt, (uint8_t)(cnt + 1)});
    //         // CanMsg msg_v = CanMsg(cnt, (uint8_t *)&chipId, 8);
    //         CanMsg msg_v = CanMsg(1, {52, 55});
    //         can1.write(msg_v);
    //         delay(1);
    //         // uart2.println("tx", msg_v.getId(),msg_v(0), msg_v(1));

    //         while(can1.pending()){
    //             uart2.println("err", can1.getTxErrCnt(), can1.getRxErrCnt(), can1.isBusOff());
    //             delay(2);
    //         }

    //         while(can1.available()){
    //             CanMsg msg_r = can1.read();
    //             uart2.println("rx", msg_r.getId(), msg_r[0], msg_r[1]);
    //         }
    //         // uart2.println(cnt++, uart2.available());
    //         cnt++;
    //         // delay(1);
    //         delay(20);
    //         // delayMicroseconds(20000);/
    //     }else{
    //         delay(10);
    //         while(can1.available()){
    //             CanMsg msg_r = can1.read();
    //             uart2.println("rx", msg_r.getId(), msg_r[0], msg_r[1]);
    //         }
    //         CanMsg msg_v = CanMsg(0, {13,14});

    //         can1.write(msg_v);


    //         // delay(20);
    //     }
    // }
}
#endif
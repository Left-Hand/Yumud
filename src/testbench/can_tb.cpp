#include "tb.h"

#include "hal/bus/can/can.hpp"
#include "sys/debug/debug_inc.h"

void can_tb(IOStream & logger, Can & can, bool tx_role){
    can.init(Can::BaudRate::Kbps125, Can::Mode::Normal);

    portC[13].outpp();

    while(1){
        if(tx_role){
            static uint8_t cnt = 0;
            CanMsg msg_v = CanMsg(1, {0x34, 0x37});
            can.write(msg_v);

            while(can.pending()){
                logger.println("err", can.getTxErrCnt(), can.getRxErrCnt(), can.isBusOff());
                delay(2);
            }

            while(can.available()){
                CanMsg msg_r = can.read();
                logger.println("rx", msg_r.id(), msg_r[0], msg_r[1]);
            }

            cnt++;
            delay(200);
            portC[13] = !portC[13];
        }else{
            while(can.available()){
                CanMsg msg_r = can.read();
                logger.println("rx", msg_r.id(), msg_r[0], msg_r[1]);
            }

            CanMsg msg_v = CanMsg(0, {0x13,0x14});
            can.write(msg_v);
        }
    }
}
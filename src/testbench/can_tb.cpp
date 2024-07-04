#include "hal/bus/can/can.hpp"
#include "sys/debug/debug_inc.h"

void can_tb(bool tx_role){
    LOGGER.init(115200);
    can1.init(Can::BaudRate::Kbps125);
    while(1){
        if(tx_role){
            static uint8_t cnt = 0;
            CanMsg msg_v = CanMsg(1, {52, 55});
            can1.write(msg_v);

            while(can1.pending()){
                LOGGER.println("err", can1.getTxErrCnt(), can1.getRxErrCnt(), can1.isBusOff());
                delay(2);
            }

            while(can1.available()){
                CanMsg msg_r = can1.read();
                LOGGER.println("rx", msg_r.id(), msg_r[0], msg_r[1]);
            }

            cnt++;
            delay(200);
            portC[13] = !portC[13];
        }else{
            while(can1.available()){
                CanMsg msg_r = can1.read();
                LOGGER.println("rx", msg_r.id(), msg_r[0], msg_r[1]);
            }
            CanMsg msg_v = CanMsg(0, {13,14});
            can1.write(msg_v);
        }
    }
}
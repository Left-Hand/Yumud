#include "tb.h"

#include "hal/bus/can/can.hpp"
#include "sys/debug/debug_inc.h"

void can_tb(IOStream & logger, Can & can, bool tx_role){
    can.init(Can::BaudRate::Mbps1, Can::Mode::Normal);

    portC[13].outpp();
    portC[14].outpp();

    {
        auto data = std::to_array<uint8_t>({3,4}); 
        uint32_t id = 0x1314;
        CanMsg msg{id, data};
        auto read = std::vector<uint8_t>(msg);
        logger.println(id, data, read);
    }

    {
        real_t data = 0.09;
        real_t data2 = 0.99;
        uint32_t id = 0x5678;
        CanMsg msg{id, data};
        // msg.load(data);
        auto read = msg.to_vector();
        logger.println(id, msg.length(), read, msg.to<real_t>());

        // auto read2 = msg.to_vector();
        auto read2 = msg.to_array<8>();
        msg.load(data2);
        logger.println(id, msg.length(), read2, msg.to<real_t>());
        for(uint8_t i = 0; i < msg.length(); i++){
            logger.println(msg[i]);
        }

        while(true);
    }

    while(1){
        if(tx_role){
            static uint8_t cnt = 0;
            CanMsg msg_v = CanMsg(1, {0x34, 0x37});
            can.write(msg_v);

            while(can.pending()){
                logger.println("err", can.getTxErrCnt(), can.getRxErrCnt(), can.isBusOff(), (int)can.getErrCode());
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
            logger.println("ava", can.available());
            while(can.available()){
                CanMsg msg_r = can.read();
                logger.println("rx", msg_r.id(), msg_r[0], msg_r[1]);
            }

            CanMsg msg_v = CanMsg(0, {0x13,0x14});
            can.write(msg_v);

            delay(200);
            portC[14] = !portC[14];
        }
    }
}
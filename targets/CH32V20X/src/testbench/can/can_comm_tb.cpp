#include "../tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/gpio/gpio_port.hpp"

void can_tb(OutputStream & logger, hal::Can & can, bool tx_role){
    can.init(1_MHz, hal::Can::Mode::Normal);

    portC[13].outpp();
    portC[14].outpp();

    {
        auto data = std::to_array<std::byte>({std::byte(3),std::byte(4)}); 
        const uint32_t id = 0x1314;
        CanMsg msg = CanMsg::from_bytes(hal::CanStdId(id), std::span(data));

        // constexpr auto a = sizeof(msg);
        auto read = msg.to_span();
        logger.println(id, data, read);

    }

    {
        real_t data = 0.09_r;
        real_t data2 = 0.99_r;
        uint32_t id = 0x5678;
        CanMsg msg = CanMsg::from_tuple(CanExtId(id), std::make_tuple(data));
        // msg.load(data);
        // auto read = msg.to_vector();
        logger.println(id, msg.size(), msg.to_span(), real_t(msg));

        // auto read2 = msg.to_vector();
        // auto read2 = msg.to_array<8>();
        msg = CanMsg::from_tuple(CanStdId(id), std::make_tuple(data2));
        logger.println(id, msg.size(), msg.to_span(), real_t(msg));
        for(uint8_t i = 0; i < msg.size(); i++){
            logger.println(msg[i]);
        }

        while(true);
    }

    while(1){
        if(tx_role){
            static uint8_t cnt = 0;
            const auto msg_v = CanMsg::from_tuple(CanStdId(1), std::make_tuple(0x34, 0x37));
            can.write(msg_v);

            while(can.pending()){
                logger.println("err", can.get_tx_errcnt(), can.get_rx_errcnt(), can.is_busoff(), (int)can.error());
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

            const auto msg_v = CanMsg::from_tuple(CanStdId(0), std::make_tuple(0x13,0x14));
            can.write(msg_v);

            delay(200);
            portC[14] = !portC[14];
        }
    }
}
#include "../tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

void can_tb(OutputStream & logger, hal::Can & can, bool is_tx){
    can.init({hal::CanBaudrate::_1M, hal::CanMode::Normal});

    hal::PC<13>().outpp();
    hal::PC<14>().outpp();

    {
        const uint32_t id = 0x1314;
        hal::CanMsg msg = hal::CanMsg::from_list(
            hal::CanStdId(id), {3,4});

        // constexpr auto a = sizeof(msg);
        auto payload = msg.iter_payload();
        logger.println(id, payload);

    }

    {
        real_t data = 0.09_r;
        real_t data2 = 0.99_r;
        uint32_t id = 0x5678;
        const auto msg = hal::CanMsg::from_bytes(
            hal::CanExtId(id), 
            std::bit_cast<std::array<uint8_t, 4>>(data.as_i32())
        );
        // msg.load(data);
        // auto read = msg.to_vector();
        logger.println(id, msg.size(), msg.iter_payload());

        // auto read2 = msg.to_vector();
        // auto read2 = msg.to_array<8>();
        const auto msg2 = hal::CanMsg::from_bytes(
            hal::CanStdId(id), 
            std::bit_cast<std::array<uint8_t, 4>>(data2.as_i32())
        );
        logger.println(id, msg2.size(), msg2.iter_payload());
        for(uint8_t i = 0; i < msg2.size(); i++){
            logger.println(msg2.iter_payload()[i]);
        }

        while(true);
    }

    while(1){
        if(is_tx){
            static uint8_t cnt = 0;
            const auto msg = hal::CanMsg::from_list(
                hal::CanStdId(1), 
                {0x34, 0x37}
            );
            can.write(msg).examine();

            while(can.pending()){
                logger.println("err", 
                    can.get_tx_errcnt(), 
                    can.get_rx_errcnt(), 
                    can.is_busoff(), 
                    can.get_last_fault()
                );
                clock::delay(2ms);
            }

            while(can.available()){
                hal::CanMsg msg_r = can.read();
                logger.println("rx", msg_r);
            }

            cnt++;
            clock::delay(200ms);
            hal::PC<13>().toggle();
        }else{
            logger.println("ava", can.available());
            while(can.available()){
                const hal::CanMsg msg_r = can.read();
                logger.println("rx", msg_r);
            }

            const auto msg = hal::CanMsg::from_list(
                hal::CanStdId(0), 
                {0x13,0x14}
            );

            can.write(msg).examine();

            clock::delay(200ms);
            hal::PC<14>().toggle();
        }
    }
}
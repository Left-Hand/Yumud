#include "../tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;

void can_tb(OutputStream & logger, hal::Can & can, bool tx_role){
    can.init({hal::CanBaudrate::_1M, hal::Can::Mode::Normal});

    portC[13].outpp();
    portC[14].outpp();

    {
        auto data = std::to_array<uint8_t>({uint8_t(3),uint8_t(4)}); 
        const uint32_t id = 0x1314;
        CanMsg msg = CanMsg::from_bytes(hal::CanStdId(id), std::span(data));

        // constexpr auto a = sizeof(msg);
        auto read = msg.payload();
        logger.println(id, data, read);

    }

    {
        real_t data = 0.09_r;
        real_t data2 = 0.99_r;
        uint32_t id = 0x5678;
        CanMsg msg = CanMsg::from_bytes(
            CanExtId::from_raw(id), 
            std::bit_cast<std::array<uint8_t, 4>>(data.to_i32())
        );
        // msg.load(data);
        // auto read = msg.to_vector();
        logger.println(id, msg.size(), msg.payload());

        // auto read2 = msg.to_vector();
        // auto read2 = msg.to_array<8>();
        const auto msg2 = CanMsg::from_bytes(
            CanStdId(id), 
            std::bit_cast<std::array<uint8_t, 4>>(data2.to_i32())
        );
        logger.println(id, msg2.size(), msg2.payload());
        for(uint8_t i = 0; i < msg2.size(); i++){
            logger.println(msg2.payload()[i]);
        }

        while(true);
    }

    while(1){
        if(tx_role){
            static uint8_t cnt = 0;
            const auto msg_v = CanMsg::from_list(
                CanStdId(1), 
                {0x34, 0x37}
            );
            can.write(msg_v);

            while(can.pending()){
                logger.println("err", 
                    can.get_tx_errcnt(), 
                    can.get_rx_errcnt(), 
                    can.is_busoff(), 
                    std::bit_cast<uint8_t>(can.get_last_error())
                );
                clock::delay(2ms);
            }

            while(can.available()){
                CanMsg msg_r = can.read();
                logger.println("rx", msg_r);
            }

            cnt++;
            clock::delay(200ms);
            portC[13].toggle();
        }else{
            logger.println("ava", can.available());
            while(can.available()){
                CanMsg msg_r = can.read();
                logger.println("rx", msg_r);
            }

            const auto msg_v = CanMsg::from_list(
                CanStdId(0), 
                {0x13,0x14}
            );
            can.write(msg_v);

            clock::delay(200ms);
            portC[14].toggle();
        }
    }
}
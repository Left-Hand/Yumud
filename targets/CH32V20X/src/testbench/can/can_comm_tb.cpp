#include "../tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

void can_tb(OutputStream & logger, hal::Can & can, bool is_tx){
    can.init({
        .remap = CAN1_REMAP,
        .mode = hal::CanMode::Normal,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs()
    });

    hal::PC<13>().outpp();
    hal::PC<14>().outpp();

    {
        const uint32_t id = 0x1314;
        hal::CanClassicMsg msg = hal::CanClassicMsg(
            hal::CanStdId::from_bits(id), 
            hal::CanClassicPayload::from_list({3,4})
        );

        // constexpr auto a = sizeof(msg);
        auto payload = msg.payload_bytes();
        logger.println(id, payload);

    }

    {
        real_t data = 0.09_r;
        real_t data2 = 0.99_r;
        uint32_t id = 0x5678;
        const auto msg = hal::CanClassicMsg(
            hal::CanExtId::from_bits(id), 
            hal::CanClassicPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(data.to_bits()))
        );
        // msg.load(data);
        // auto read = msg.to_vector();
        logger.println(id, msg.length(), msg.payload_bytes());

        // auto read2 = msg.to_vector();
        // auto read2 = msg.to_array<8>();
        const auto msg2 = hal::CanClassicMsg(
            hal::CanExtId::from_bits(id), 
            hal::CanClassicPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(data2.to_bits()))
        );
        logger.println(id, msg2.length(), msg2.payload_bytes());
        for(uint8_t i = 0; i < msg2.length(); i++){
            logger.println(msg2.payload_bytes()[i]);
        }

        while(true);
    }

    while(1){
        if(is_tx){
            static uint8_t cnt = 0;
            const auto msg = hal::CanClassicMsg(
                hal::CanStdId::from_bits(1), 
                hal::CanClassicPayload::from_list({0x34, 0x37})
            );
            can.write(msg).examine();

            while(can.pending()){
                logger.println("err", 
                    can.get_tx_errcnt(), 
                    can.get_rx_errcnt(), 
                    can.is_busoff(), 
                    can.last_exception()
                );
                clock::delay(2ms);
            }

            while(can.available()){
                hal::CanClassicMsg msg_r = can.read();
                logger.println("rx", msg_r);
            }

            cnt++;
            clock::delay(200ms);
            hal::PC<13>().toggle();
        }else{
            logger.println("ava", can.available());
            while(can.available()){
                const hal::CanClassicMsg msg_r = can.read();
                logger.println("rx", msg_r);
            }

            const auto msg = hal::CanClassicMsg(
                hal::CanStdId::from_bits(0), 
                hal::CanClassicPayload::from_list({0x13,0x14})
            );

            can.write(msg).examine();

            clock::delay(200ms);
            hal::PC<14>().toggle();
        }
    }
}
#include "../tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

void can_tb(OutputStream & logger, hal::Can & can, bool is_tx){
    can.init({
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M)
    });

    hal::PC<13>().outpp();
    hal::PC<14>().outpp();

    {
        const uint32_t id = 0x1314;
        hal::BxCanFrame frame = hal::BxCanFrame(
            hal::CanStdId::from_bits(id), 
            hal::BxCanPayload::from_list({3,4})
        );

        // constexpr auto a = sizeof(frame);
        auto payload = frame.payload_bytes();
        logger.println(id, payload);

    }

    {
        real_t data = 0.09_r;
        real_t data2 = 0.99_r;
        uint32_t id = 0x5678;
        const auto frame = hal::BxCanFrame(
            hal::CanExtId::from_bits(id), 
            hal::BxCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(data.to_bits()))
        );
        // frame.load(data);
        // auto read = frame.to_vector();
        logger.println(id, frame.length(), frame.payload_bytes());

        // auto read2 = frame.to_vector();
        // auto read2 = frame.to_array<8>();
        const auto frame2 = hal::BxCanFrame(
            hal::CanExtId::from_bits(id), 
            hal::BxCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(data2.to_bits()))
        );
        logger.println(id, frame2.length(), frame2.payload_bytes());
        for(uint8_t i = 0; i < frame2.length(); i++){
            logger.println(frame2.payload_bytes()[i]);
        }

        while(true);
    }

    while(1){
        if(is_tx){
            static uint8_t cnt = 0;
            const auto frame = hal::BxCanFrame(
                hal::CanStdId::from_bits(1), 
                hal::BxCanPayload::from_list({0x34, 0x37})
            );
            can.try_write(frame).examine();


            logger.println("err", 
                can.get_tx_errcnt(), 
                can.get_rx_errcnt(), 
                can.is_busoff(), 
                can.last_error()
            );
            clock::delay(2ms);
            // }

            while(can.available()){
                hal::BxCanFrame frame_r = can.read();
                logger.println("rx", frame_r);
            }

            cnt++;
            clock::delay(200ms);
            hal::PC<13>().write(~hal::PC<13>().read());
        }else{
            logger.println("ava", can.available());
            while(can.available()){
                const hal::BxCanFrame frame_r = can.read();
                logger.println("rx", frame_r);
            }

            const auto frame = hal::BxCanFrame(
                hal::CanStdId::from_bits(0), 
                hal::BxCanPayload::from_list({0x13,0x14})
            );

            can.try_write(frame).examine();

            clock::delay(200ms);
            hal::PC<14>().toggle();
        }
    }
}
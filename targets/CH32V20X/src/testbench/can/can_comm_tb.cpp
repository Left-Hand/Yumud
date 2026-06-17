#include "../tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

#include "hal/conn/can/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

void can_tb( hal::Can & can, bool is_tx){
    can.init({
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanNominalBitTimming(hal::CanBaudrate::_1M)
    });

    {
        const uint32_t id = 0x1314;
        const auto frame = hal::ClassicCanFrame::from_parts(
            hal::CanStdId::from_bits(id), 
            hal::ClassicCanPayload::from_list({3,4})
        );

        // constexpr auto a = sizeof(frame);
        auto payload = frame.payload_bytes();
        DEBUG_PRINTLN(id, payload);

    }

    {
        iq16 data = 0.09_r;
        iq16 data2 = 0.99_r;
        uint32_t id = 0x5678;
        const auto frame = hal::ClassicCanFrame::from_parts(
            hal::CanExtId::from_bits(id), 
            hal::ClassicCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(data.to_bits()))
        );
        // frame.load(data);
        // auto read = frame.to_vector();
        DEBUG_PRINTLN(id, frame.length(), frame.payload_bytes());

        // auto read2 = frame.to_vector();
        // auto read2 = frame.to_array<8>();
        const auto frame2 = hal::ClassicCanFrame::from_parts(
            hal::CanExtId::from_bits(id), 
            hal::ClassicCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(data2.to_bits()))
        );
        DEBUG_PRINTLN(id, frame2.length(), frame2.payload_bytes());
        for(uint8_t i = 0; i < frame2.length(); i++){
            DEBUG_PRINTLN(frame2.payload_bytes()[i]);
        }

        while(true);
    }

    while(1){
        if(is_tx){
            static uint8_t cnt = 0;
            const auto frame = hal::ClassicCanFrame::from_parts(
                hal::CanStdId::from_bits(1), 
                hal::ClassicCanPayload::from_list({0x34, 0x37})
            );
            can.try_write(frame).examine();


            DEBUG_PRINTLN("err", 
                can.get_tx_errcnt(), 
                can.get_rx_errcnt(), 
                can.is_busoff(), 
                can.last_error()
            );
            clock::delay(2ms);
            // }

            while(can.available()){
                hal::ClassicCanFrame frame_r = can.try_read().unwrap().clone();
                DEBUG_PRINTLN("rx", frame_r);
            }

            cnt++;
            clock::delay(200ms);
            hal::PC<13>().write(~hal::PC<13>().read());
        }else{
            DEBUG_PRINTLN("ava", can.available());
            while(can.available()){
                const hal::ClassicCanFrame frame_r = can.try_read().unwrap().clone();
                DEBUG_PRINTLN("rx", frame_r);
            }

            const auto frame = hal::ClassicCanFrame::from_parts(
                hal::CanStdId::from_bits(0), 
                hal::ClassicCanPayload::from_list({0x13,0x14})
            );

            can.try_write(frame).examine();

            clock::delay(200ms);
            hal::PC<14>().toggle();
        }
    }
}
#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/can/hw_singleton.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/hw_singleton.hpp"


using namespace ymd;

void can_ring_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
        .tx_strategy = CommStrategy::Blocking
    });

    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    
    auto led = hal::PC<14>();
    led.outpp(HIGH);

    auto & can = hal::can1;
    can.init({
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Loopback,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M)
    });

    auto write_frame = [&](const hal::BxCanFrame & frame){
        DEBUG_PRINTLN("tx", frame);
        return can.try_write(frame);
    };

    static constexpr auto UNREACHABLE_FRAMES = std::to_array({
        hal::BxCanFrame(
            hal::CanStdId::from_bits(0x100), 
            hal::BxCanPayload::from_list({0, 1, 3})
        ),
        hal::BxCanFrame(
            hal::CanStdId::from_bits(0x300), 
            hal::BxCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(0x12345678))
        ),
        hal::BxCanFrame(
            hal::CanExtId::from_bits(0x400), 
            hal::BxCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(0x12345678))
        )
    });

    static constexpr auto REACHABLE_FRAMES = std::to_array({
        hal::BxCanFrame(
            hal::CanStdId::from_bits(0x200), 
            hal::BxCanPayload::from_list({0, 1, 2})
        ),
        hal::BxCanFrame(
            hal::CanStdId::from_bits(0x200), 
            hal::BxCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(0x12345678))
        ),
    });

    while(true){
        for(const auto frame : UNREACHABLE_FRAMES){
            write_frame(frame).examine();
            clock::delay(2ms);
        }

        for(const auto frame : REACHABLE_FRAMES){
            write_frame(frame).examine();
            clock::delay(2ms);
        }

        clock::delay(10ms);

        if(can.available()){
            DEBUG_PRINTLN(can.available());
            while(can.available()){
                auto rx_frame = can.read();
                DEBUG_PRINTLN("rx", rx_frame);
            }
        }else{
            DEBUG_PRINTLN("no frame received");
        }

        clock::delay(200ms);
    }

}
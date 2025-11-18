#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

void can_ring_main(){
    DEBUGGER_INST.init({
        .baudrate = 576000, 
        .tx_strategy = CommStrategy::Blocking
    });

    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    
    auto led = hal::PC<14>();
    led.outpp(HIGH);

    auto & can = hal::can1;
    can.init({
        .remap = CAN1_REMAP,
        .mode = hal::Can::Mode::Loopback,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs()
    });

    auto write_msg = [&](const hal::CanClassicMsg & msg){
        DEBUG_PRINTLN("tx", msg);
        return can.write(msg);
    };

    static constexpr auto UNREACHABLE_MSGS = std::to_array({
        hal::CanClassicMsg::from_list(hal::CanStdId(0x100), {0, 1, 3}),
        hal::CanClassicMsg::from_bytes(hal::CanStdId(0x300), std::bit_cast<std::array<uint8_t, 4>>(0x12345678)),
        hal::CanClassicMsg::from_bytes(hal::CanExtId(0x400), std::bit_cast<std::array<uint8_t, 4>>(0x12345678))
    });

    static constexpr auto REACHABLE_MSGS = std::to_array({
        hal::CanClassicMsg::from_list(hal::CanStdId(0x200), {0, 1, 2}),
        hal::CanClassicMsg::from_bytes(hal::CanStdId(0x200), std::bit_cast<std::array<uint8_t, 4>>(0x12345678)),
    });

    while(true){
        for(const auto msg : UNREACHABLE_MSGS){
            write_msg(msg).examine();
            clock::delay(2ms);
        }

        for(const auto msg : REACHABLE_MSGS){
            write_msg(msg).examine();
            clock::delay(2ms);
        }

        clock::delay(10ms);

        if(can.available()){
            DEBUG_PRINTLN(can.available());
            while(can.available()){
                auto rx_msg = can.read();
                DEBUG_PRINTLN("rx", rx_msg);
            }
        }else{
            DEBUG_PRINTLN("no msg received");
        }

        clock::delay(200ms);
    }

}
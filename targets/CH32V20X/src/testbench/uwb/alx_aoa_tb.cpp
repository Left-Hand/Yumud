#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "drivers/Proximeter/ALX_AOA/alx_aoa_prelude.hpp"

using namespace ymd;
using drivers::AlxAoa_Prelude;

// #define DEBUGGER_INST hal::uart2
[[nodiscard]] static constexpr uint8_t xor_bytes(
    const std::span<const uint8_t> bytes
){
    uint8_t ret = 0;
    for(const auto byte : bytes){
        ret ^= byte;
    }
    return ret;
}


void alx_aoa_main(){
    DEBUGGER_INST.init({
        576000 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);

    auto & alx_uart = hal::uart1;

    using AlxEvent = AlxAoa_Prelude::Event;
    using AlxError = AlxAoa_Prelude::Error;

    auto alx_ev_handler = [&](const Result<AlxEvent, AlxError> & res){ 
        if(res.is_ok()){
            const auto & ev = res.unwrap();
            DEBUG_PRINTLN("alx_ev", ev);
        }else{
            const auto & err = res.unwrap_err();
            DEBUG_PRINTLN("alx_err", err);

        }
    };

    auto alx_parser = drivers::AlxAoa_StreamParser(alx_ev_handler);
    alx_uart.init({
        AlxAoa_Prelude::DEFAULT_UART_BUAD
    });

    auto tx_led = hal::PC<13>();
    auto rx_led = hal::PC<14>();
    tx_led.outpp();
    rx_led.outpp();

    // alx_uart.set_event_callback([&](const hal::UartEvent& ev){
    //     switch(ev.kind()){
    //         case hal::UartEvent::RxIdle:
    //             rx_led.set();
    //             clock::delay(1ms);
    //             rx_led.clr();
    //             break;
    //         case hal::UartEvent::TxIdle:
    //             tx_led.set();
    //             clock::delay(1ms);
    //             tx_led.clr();
    //             break;
    //         default:
    //             PANIC{"Unexpected event", ev};
    //             break;
    //     }
    // });

    const auto bytes = std::to_array<uint8_t>({
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x25, 0x00, 0x0B, 
        0x20, 0x01, 0x01, 0x00, 0x00, 0x00, 0xAA, 0xA2, 
        0x00, 0x00, 0xAA, 0xA1, 0x00, 0x00, 0x00, 0x19, 
        0x00, 0x12, 0xFF, 0xCA, 0x12, 0x34, 0x00, 0x0B, 
        0x00, 0x00, 0x00, 0x00, 0x1E,
    });

    const auto header_bytes = std::span(bytes.data(), 6);
    const auto body_bytes = std::span(bytes.data() + 6, std::prev(bytes.end()));
    DEBUG_PRINTLN(
        "xor_acc", 
        xor_bytes(header_bytes), 
        xor_bytes(body_bytes),
        xor_bytes(header_bytes) ^ xor_bytes(body_bytes),
        0x1e
    );
    clock::delay(1ms);
    
    for(const auto byte : bytes){
        alx_parser.push_byte(byte);
        clock::delay(1ms);
    }

    const auto bytes2 = std::to_array<uint8_t>({
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
        0x10, 0x00, 0x0B, 0x20, 0x02, 
        0x01, 0x01, 0x00, 0x00, 0xAA, 0xA1
    });

    for(const auto byte : bytes2){
        alx_parser.push_byte(byte);
        clock::delay(1ms);
    };
    PANIC{"done"};
    while(true){

    }


}
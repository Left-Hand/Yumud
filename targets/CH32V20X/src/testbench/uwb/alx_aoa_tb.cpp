#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "drivers/Proximeter/ALX_AOA/alx_aoa_prelude.hpp"
#include "core/sync/timer.hpp"

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
    

    #if defined(CH32V20X)
    auto & alx_uart = hal::uart1;
    #elif defined(CH32V30X)
    auto & alx_uart = hal::uart2;
    #else
    static_assert(false, "Unsupported MCU");
    #endif

    using AlxEvent = AlxAoa_Prelude::Event;
    using AlxError = AlxAoa_Prelude::Error;

    using AlxLocation = drivers::AlxAoa_Prelude::Location;
    using AlxHeartBeat = drivers::AlxAoa_Prelude::HeartBeat;

    auto alx_ev_handler = [&](const Result<AlxEvent, AlxError> & res){ 
        if(res.is_ok()){

            const auto & ev = res.unwrap();
            if(ev.is<AlxLocation>()){
                const AlxLocation & loc = ev.unwrap_as<AlxLocation>();
                DEBUG_PRINTLN(
                    loc.distance.to_meters(), 
                    loc.azimuth.to_angle().to_degrees(), 
                    loc.elevation.to_angle().to_degrees()
                );
            }else if(ev.is<AlxHeartBeat>()){
                DEBUG_PRINTLN("alx_heartBeat", ev.unwrap_as<AlxHeartBeat>());
            }
        }else{
            [[maybe_unused]] const auto & err = res.unwrap_err();
            // DEBUG_PRINTLN("alx_err", err);

        }
    };

    auto alx_parser = drivers::AlxAoa_StreamParser(alx_ev_handler);
    alx_uart.init({
        AlxAoa_Prelude::DEFAULT_UART_BUAD
    });


    auto red_led_gpio_ = hal::PC<13>();
    auto blue_led_gpio_ = hal::PC<14>();
    red_led_gpio_.outpp();
    blue_led_gpio_.outpp();

    auto blink_service_poller = [&]{

        red_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        blue_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
    };

    #if 0
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
    #endif

    uint32_t received_bytes_cnt_ = 0;
    while(true){
        auto collect_bytes = []{
            std::vector<uint8_t> bytes;
            if(alx_uart.available()){
                char chr;
                alx_uart.read1(chr);
                bytes.push_back(uint8_t(chr));
            }
            return bytes;
        };

        const auto bytes = collect_bytes();

        alx_parser.push_bytes(std::span(bytes)); 
        received_bytes_cnt_+=bytes.size();
        // DEBUG_PRINTLN("alx_uart_rx", uint8_t(chr));


        blink_service_poller();

        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        
        report_timer.invoke_if([&]{
            // if(bytes.size() == 0) return;
            // if(DEBUGGER.pending() != 0) return;
            // PANIC{bytes};
            // DEBUG_PRINTLN_IDLE(
            //     received_bytes_cnt_
            // );
            // for(const auto byte : bytes){
            //     DEBUG_PRINT(byte);
            // }
    });
    }


}
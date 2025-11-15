#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "drivers/Proximeter/M10/M10_prelude.hpp"
#include "core/sync/timer.hpp"

using namespace ymd;
using drivers::M10_Prelude;
using drivers::M10_StreamParser;

#if 0

void m10_main(){
    DEBUGGER_INST.init({
        576000 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    

    #if defined(CH32V20X)
    auto & m10_uart = hal::uart1;
    #elif defined(CH32V30X)
    auto & m10_uart = hal::uart2;

    #else
    static_assert(false, "Unsupported MCU");
    #endif

    using M10Event = M10_Prelude::Event;


    auto m10_ev_handler = [&](const M10Event & ev){ 
        if(ev.is<M10Event::DataReady>()){
            const auto & packet = ev.unwrap_as<M10Event::DataReady>().frame;
            // for(const auto & points: packet.points){
            //     DEBUG_PRINTLN();
            // }
            DEBUG_PRINTLN(packet.points[0], packet.points[11]);
        }else if(ev.is<M10Event::InvalidCrc>()){
            DEBUG_PRINTLN(ev.unwrap_as<M10Event::InvalidCrc>());
            PANIC{};
        }
        // DEBUG_PRINTLN(ev.dist_cm, ev.signal_strength.to_dbm());
    };

    auto m10_parser = drivers::M10_StreamParser(m10_ev_handler);
    m10_uart.init({
        M10_Prelude::DEFAULT_UART_BAUD
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


    uint32_t received_bytes_cnt_ = 0;
    while(true){
        auto collect_bytes = [&]{
            
            #if 1
            std::vector<uint8_t> bytes(8);
            while(m10_uart.available()){
                char chr;
                m10_uart.read1(chr);
                bytes.push_back(uint8_t(chr));
            }

            #else
            const auto bytes = std::to_array<uint8_t>({
                0x54, 0x2C, 0x68, 0x08, 0xAB, 0x7E, 0xE0, 0x00, 
                0xE4, 0xDC, 0x00, 0xE2, 0xD9, 0x00, 0xE5, 0xD5, 
                0x00, 0xE3, 0xD3, 0x00, 0xE4, 0xD0, 0x00, 0xE9, 
                0xCD, 0x00, 0xE4, 0xCA, 0x00, 0xE2, 0xC7, 0x00, 
                0xE9, 0xC5, 0x00, 0xE5, 0xC2, 0x00, 0xE5, 0xC0, 
                0x00, 0xE5, 0xBE, 0x82, 0x3A, 0x1A, 
                0x50, 
            });

            [[maybe_unused]] static constexpr auto bytes_size = sizeof(bytes);

            #endif

            // if(bytes.size()) DEBUG_PRINTLN(bytes);
            return bytes;
        };

        if(m10_uart.available()){
            // const auto u_begin = clock::micros();
            const auto bytes = collect_bytes();
            m10_parser.push_bytes(std::span(bytes)); 
            received_bytes_cnt_+=bytes.size();
            // DEBUG_PRINTLN(clock::micros() - u_begin);
        }
        // DEBUG_PRINTLN("m10_uart_rx", uint8_t(chr));


        blink_service_poller();


        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        
        report_timer.invoke_if([&]{
            DEBUG_PRINTLN_IDLE(received_bytes_cnt_);
            // if(bytes.size() == 0) return;
            // if(DEBUGGER.pending() != 0) return;
            // PANIC{bytes};

            // for(const auto byte : bytes){
            //     DEBUG_PRINT(byte);
            // }
    });
    }
}


#endif
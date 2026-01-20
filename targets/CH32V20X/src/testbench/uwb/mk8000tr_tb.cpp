#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "drivers/Proximeter/MK8000TR/mk8000tr_stream.hpp"
#include "core/async/timer.hpp"

using namespace ymd;

using drivers::mk8000tr::MK8000TR_ParseReceiver;

void mk8000tr_main(){
    DEBUGGER_INST.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(576_KHz), 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    

    #if defined(CH32V20X)
    auto & mk8000tr_uart_ = hal::usart1;
    #elif defined(CH32V30X)
    auto & mk8000tr_uart_ = hal::usart2;
    #else
    static_assert(false, "Unsupported MCU");
    #endif

    using MkEvent = drivers::mk8000tr::Event;


    auto mk8000tr_ev_handler = [&](const MkEvent & ev){ 
        DEBUG_PRINTLN(ev.dist_cm, ev.signal_strength.to_dbm());
    };

    auto mk8000tr_parser = MK8000TR_ParseReceiver(mk8000tr_ev_handler);
    mk8000tr_uart_.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(576_KHz), 
    });


    auto red_led_pin_ = hal::PC<13>();
    auto blue_led_pin_ = hal::PC<14>();
    red_led_pin_.outpp();
    blue_led_pin_.outpp();

    auto blink_service_poller = [&]{

        red_led_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        blue_led_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
    };


    uint32_t received_bytes_cnt_ = 0;
    while(true){
        auto collect_bytes = [&]{
            std::vector<uint8_t> bytes(8);

            #if 1
            while(mk8000tr_uart_.available()){
                char chr;
                const auto read_len =  mk8000tr_uart_.try_read_char(chr);
                if(read_len == 0) break;
                bytes.push_back(uint8_t(chr));
            }
            // DEBUG_PRINTLN_IDLE(
            //     received_bytes_cnt_
            // );

            #else
            auto exmaple = std::to_array<uint8_t>({
                0xf0, 0x05, 0x05, 0x00, 0x2d, 0x00, 0xb7, 0xaa
            });

            for(auto byte : exmaple){
                bytes.push_back(byte);
            }
            #endif

            // if(bytes.size()) DEBUG_PRINTLN(bytes);
            return bytes;
        };

        if(mk8000tr_uart_.available()){
            // const auto u_begin = clock::micros();
            const auto bytes = collect_bytes();
            mk8000tr_parser.push_bytes(std::span(bytes)); 
            received_bytes_cnt_+=bytes.size();
            // DEBUG_PRINTLN(clock::micros() - u_begin);
        }
        // DEBUG_PRINTLN("mk8000tr_uart_rx", uint8_t(chr));


        blink_service_poller();


        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        
        report_timer.invoke_if([&]{
            // if(bytes.size() == 0) return;
            // if(DEBUGGER.pending() != 0) return;
            // PANIC{bytes};

            // for(const auto byte : bytes){
            //     DEBUG_PRINT(byte);
            // }
    });
    }


}
#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "drivers/Proximeter/M10/M10_prelude.hpp"
#include "core/async/timer.hpp"

using namespace ymd;
using drivers::m10::M10_ParserSink;


void m10_main(){
    DEBUGGER_INST.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 
    });

    DEBUGGER.retarget(&DEBUGGER_INST);

    #if defined(CH32V20X)
    auto & m10_uart_ = hal::uart1;
    #elif defined(CH32V30X)
    auto & m10_uart_ = hal::uart2;

    #else
    static_assert(false, "Unsupported MCU");
    #endif

    using M10Event = drivers::m10::Event;

    uq24 last_turns_ = 0;
    auto m10_ev_handler = [&](const M10Event & ev){ 
        if(ev.is<M10Event::DataReady>()){
            const auto & packet = ev.unwrap_as<M10Event::DataReady>().sector;
            const auto turns = packet.start_angle.to_turns();
            const auto spin_rate = packet.spin_speed.to_tps();
            const auto meters = packet.distances[0].to_meters();

            DEBUG_PRINTLN(
                turns - last_turns_,
                spin_rate,
                meters
            );

            last_turns_ = turns;
        }else if(ev.is<M10Event::InvalidCrc>()){
            DEBUG_PRINTLN(ev.unwrap_as<M10Event::InvalidCrc>());
            PANIC{};
        }
    };

    auto m10_parser = M10_ParserSink(m10_ev_handler);
    m10_uart_.init({
        .remap = hal::UART1_REMAP_PA9_PA10,
        .baudrate = drivers::m10::DEFAULT_UART_BAUD
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

    m10_uart_.set_event_handler([&](const hal::UartEvent ev){
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:{
                while(m10_uart_.available()){
                    char chr;
                    const auto read_len = m10_uart_.try_read_char(chr);
                    if(read_len == 0) break;
                    m10_parser.push_byte(chr); 
                }
            }
                break;
            case hal::UartEvent::RxOverrun:{
                PANIC{};
            }
                break;
            default:
                break;
        }
    });



    while(true){


        blink_service_poller();

        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        report_timer.invoke_if([&]{
            // DEBUG_PRINTLN_IDLE(received_bytes_cnt_);
            // if(bytes.size() == 0) return;
            // if(DEBUGGER.pending() != 0) return;
            // PANIC{bytes};

            // for(const auto byte : bytes){
            //     DEBUG_PRINT(byte);
            // }
    });
    }
}

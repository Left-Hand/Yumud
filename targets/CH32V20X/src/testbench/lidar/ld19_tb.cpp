#include "src/testbench/tb.h"

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "drivers/Proximeter/LD19/LD19.hpp"
#include "core/async/timer.hpp"

using namespace ymd;
using namespace ymd::drivers;


void ld19_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        .tx_strategy = CommStrategy::Blocking
    });
    DEBUGGER.retarget(&DEBUGGER_INST);



    using LD19Event = ld19::Event;


    auto ld19_ev_handler = [&](const LD19Event & ev){
        if(ev.is<LD19Event::DataReady>()){
            const auto & packet = ev.unwrap_as<LD19Event::DataReady>().packet;
            // for(const auto & points: packet.points){
            //     DEBUG_PRINTLN();
            // }
            DEBUG_PRINTLN(packet.points[0], packet.points[11]);
        }else if(ev.is<LD19Event::InvalidCrc>()){
            DEBUG_PRINTLN(ev.unwrap_as<LD19Event::InvalidCrc>());
            PANIC{};
        }
        // DEBUG_PRINTLN(ev.dist_cm, ev.signal_strength.to_dbm());
    };

    auto ld19_parser_ = ld19::LD19_ParseReceiver(ld19_ev_handler);



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

    #if defined(CH32V20X)
    auto & ld19_uart_ = hal::usart1;
    ld19_uart_.init({
        .remap = hal::USART1_REMAP_PA9_PA10,
        .baudrate = hal::NearestFreq(ld19::DEFAULT_UART_BAUD)
    });

    #elif defined(CH32V30X)
    auto & ld19_uart_ = hal::uart4;
    ld19_uart_.init({
        .remap = hal::UART4_REMAP_PE0_PE1,
        .baudrate = hal::NearestFreq(ld19::DEFAULT_UART_BAUD)
    });
    #else
    static_assert(false, "Unsupported MCU");
    #endif

    ld19_uart_.set_event_callback([&](const hal::UartEvent & ev){
        auto poll_parser = [&](){
            while(true){
                uint8_t byte;
                if(ld19_uart_.try_read_byte(byte) == 0) break;
                ld19_parser_.push_byte(static_cast<uint8_t>(byte));
            }
        };
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:
                poll_parser();
                ld19_parser_.reset();
                break;
            case hal::UartEvent::RxBulk:
                poll_parser();
                break;
            default:
                break;
        }
    });

    while(true){
        blink_service_poller();

        static auto report_timer = async::RepeatTimer::from_duration(3ms);
        report_timer.invoke_if([&]{
            DEBUG_PRINTLN_IDLE(received_bytes_cnt_);

    });
    }


}

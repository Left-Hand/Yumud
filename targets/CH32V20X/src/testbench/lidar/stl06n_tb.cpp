#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "drivers/Proximeter/STL06N/stl06n.hpp"
#include "core/async/timer.hpp"

using namespace ymd;
using namespace ymd::drivers;


void stl06n_main(){
    DEBUGGER_INST.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    

    #if defined(CH32V20X)
    auto & stl06n_uart_ = hal::usart1;
    #elif defined(CH32V30X)
    auto & stl06n_uart_ = hal::usart2;

    #else
    static_assert(false, "Unsupported MCU");
    #endif

    using LidarEvent = stl06n::Event;


    auto lidar_ev_handler = [&](const LidarEvent & ev){ 
        if(ev.is<LidarEvent::DataReady>()){
            const auto & sector = ev.unwrap_as<LidarEvent::DataReady>().sector;
            DEBUG_PRINTLN(sector.points[0], sector.points[11]);
        }else if(ev.is<LidarEvent::InvalidCrc>()){
            DEBUG_PRINTLN(ev.unwrap_as<LidarEvent::InvalidCrc>());
            PANIC{};
        }
        // DEBUG_PRINTLN(ev.dist_cm, ev.signal_strength.to_dbm());
    };

    auto stl06n_parser = stl06n::STL06N_ParserSink(lidar_ev_handler);

    stl06n_uart_.init({
        .remap = hal::UART1_REMAP_PA9_PA10,
        .baudrate = stl06n::DEFAULT_UART_BAUD
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


    stl06n_uart_.set_event_handler([&](const hal::UartEvent ev){
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:{
                while(stl06n_uart_.available()){
                    char chr;
                    const auto read_len = stl06n_uart_.try_read_char(chr);
                    if(read_len == 0) break;
                    stl06n_parser.push_byte(chr); 
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
            DEBUG_PRINTLN_IDLE(received_bytes_cnt_);
        });
    }


}
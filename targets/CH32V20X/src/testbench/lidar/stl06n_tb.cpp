#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/async/timer.hpp"

#include "drivers/Proximeter/STL06N/stl06n.hpp"

using namespace ymd;
using namespace ymd::drivers;


void stl06n_main(){

    #if defined(CH32V20X)
    auto & UART = hal::usart2;
    UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking
    });
    #elif defined(CH32V30X)
    auto & UART = hal::uart6;
    UART.init({
        .remap = hal::UART6_REMAP_PC0_PC1,
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking
    });
    #endif
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    DEBUGGER.no_fieldname(EN);

    using LidarEvent = stl06n::Event;
    using LidarPoint = stl06n::LidarPoint;

    auto watch_pin_ = hal::PA<11>();
    watch_pin_.set_mode(hal::GpioMode::OutPP);
    watch_pin_.set_low();

    volatile size_t lidar_recv_bytes_count_ = 0;
    volatile size_t lidar_sector_count_ = 0;
    volatile size_t lidar_ev_count_ = 0;
    volatile size_t lidar_crc_err_count_ = 0;

    std::array<LidarPoint, 12> points;
    Microseconds lidar_clone_elapsed_us_ = 0us;
    auto lidar_ev_handler = [&](const LidarEvent & ev){
        watch_pin_.set_high();
        watch_pin_.set_low();
        watch_pin_.set_high();
        auto guard = make_scope_guard([&]{
            watch_pin_.set_low();
        });

        lidar_ev_count_++;
        if(ev.is<LidarEvent::DataReady>()){
            const auto & sector = ev.unwrap_as<LidarEvent::DataReady>().sector;
            lidar_sector_count_++;

            lidar_clone_elapsed_us_ = measure_total_elapsed_us([&]{
                sector.points.clone_to(std::span(points));
            });
        }else if(ev.is<LidarEvent::InvalidCrc>()){
            lidar_crc_err_count_++;
            // DEBUG_PRINTLN("INVALID CRC", ev.unwrap_as<LidarEvent::InvalidCrc>());
            // PANIC{};
        }
        // DEBUG_PRINTLN(ev.dist_cm, ev.signal_strength.to_dbm());
    };

    auto stl06n_parser_ = stl06n::STL06N_ParseReceiver(lidar_ev_handler);


    #if defined(CH32V20X)
    auto & stl06n_uart_ = hal::uart4;
    stl06n_uart_.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(230400),
        .tx_strategy = CommStrategy::Blocking
    });
    #elif defined(CH32V30X)
    auto & stl06n_uart_ = hal::uart4;
    stl06n_uart_.init({
        .remap = hal::UartRemap::_3,
        .baudrate = hal::NearestFreq(230400),
        .tx_strategy = CommStrategy::Blocking
    });
    #else
    static_assert(false, "Unsupported MCU");
    #endif


    stl06n_uart_.set_event_handler([&](const hal::UartEvent & ev){
        watch_pin_.set_high();
        auto guard = make_scope_guard([&]{
            watch_pin_.set_low();
        });

        lidar_recv_bytes_count_++;

        #if 1
        auto poll_parser = [&](){
            while(true){
                uint8_t byte;
                if(stl06n_uart_.try_read_byte(byte) == 0) break;
                stl06n_parser_.push_byte(static_cast<uint8_t>(byte));
            }
        };

        switch(ev.kind()){
            case hal::UartEvent::RxIdle:
                poll_parser();
                stl06n_parser_.reset();
                break;
            case hal::UartEvent::RxBulk:
                poll_parser();
                break;
            default:
                break;
        }
        #else
        while(true){
            char chr;
            if(stl06n_uart_.try_read_byte(chr) == 0) break;
            // stl06n_parser_.push_byte(static_cast<uint8_t>(chr));
        }
        #endif
    });

    #if 0
    const uint8_t bytes[] = {
        0x54, 0x2c, 0xfe, 0xd, 0x79, 0x21, 0x55, 0x02, 0x14, 0x55, 0x02, 0x14,
        0x57, 0x02, 0x14, 0x6a, 0x02, 0x14, 0xe4, 0x02, 0x5c, 0xe0, 0x02, 0x2c,
        0xe0, 0x02, 0x1e, 0xe3, 0x02, 0x38, 0x25, 0x00, 0x60, 0x27, 0x00, 0x14,
        0x2b, 0x00, 0x14, 0x30, 0x00, 0x13, 0xa6, 0x24, 0x97, 0x5a, 0xbc
    };

    for(size_t i = 0; i < std::size(bytes); i++){
        stl06n_parser_.push_byte(bytes[i]);

        DEBUG_PRINTLN(
            lidar_ev_count_,
            lidar_crc_err_count_,
            lidar_sector_count_,
            static_cast<uint8_t>(stl06n_parser_.fsm_state_),
            static_cast<size_t>(stl06n_parser_.bytes_count_)
        );
        DEBUG_PRINTLN("------------");
    }

    PANIC{};
    #endif

    [[maybe_unused]] static auto report_timer = async::RepeatTimer::from_duration(3ms);
    while(true){

        DEBUG_PRINTLN(
            clock::millis().count(),
            // hal::uart6.available(),
            UART.available(),
            hal::uart4.available(),
            // static_cast<uint8_t>(stl06n_parser_.fsm_state_),
            // static_cast<size_t>(stl06n_parser_.bytes_count_)
            lidar_recv_bytes_count_,
            lidar_crc_err_count_,
            lidar_sector_count_,
            lidar_clone_elapsed_us_.count()
            // hal::PE<1>().read().to_bool()
        );
        // report_timer.invoke_if([&]{
        // });
    }


}

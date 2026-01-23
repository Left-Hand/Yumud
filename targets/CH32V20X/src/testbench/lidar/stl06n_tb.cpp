#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/utils/zero.hpp"
#include "core/async/timer.hpp"

#include "core/mem/o1heap/o1heap_alloc.hpp"
#include "core/string/view/string_view.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Proximeter/STL06N/stl06n.hpp"

#include <ranges>



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
    using stl06n::PackedLidarPoint;

    auto watch_pin_ = hal::PA<11>();
    watch_pin_.set_mode(hal::GpioMode::OutPP);
    watch_pin_.set_low();

    volatile size_t lidar_sector_count_ = 0;
    volatile size_t lidar_ev_count_ = 0;
    volatile size_t lidar_crc_err_count_ = 0;

    std::array<PackedLidarPoint, 12> points;
    Microseconds lidar_clone_elapsed_us_ = 0us;
    Angular<uq32> last_start_angle_ = Zero;
    Angular<uq32> last_stop_angle_ = Zero;
    PackedLidarPoint point_ = Zero;
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

            sector.packed_cluster.clone_to(std::span(points));
            // auto && view = make_std_range(sector.packed_cluster.iter())
            //     | std::views::take(2);

            const auto start_angle = sector.start_angle_code.to_angle();
            const auto stop_angle = sector.stop_angle_code.to_angle();
            if(last_start_angle_ > start_angle){
                lidar_sector_count_ = 0;
                point_ = sector.packed_cluster[0];
            }else{
                lidar_sector_count_++;
            }

            last_start_angle_ = start_angle;
            last_stop_angle_ = stop_angle;
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


    stl06n_uart_.set_event_callback([&](const hal::UartEvent & ev){
        watch_pin_.set_high();
        auto guard = make_scope_guard([&]{
            watch_pin_.set_low();
        });

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
        // const auto heap_alloc_elapsed_us = measure_total_elapsed_us([&]{
        //     auto arena = std::make_unique<uint8_t[]>(128 * 64);
        // });
        DEBUG_PRINTLN(
            clock::millis().count(),
            // static_cast<uint8_t>(stl06n_parser_.fsm_state_),
            // static_cast<size_t>(stl06n_parser_.bytes_count_)
            lidar_clone_elapsed_us_.count(),
            last_start_angle_.to_turns(),
            last_stop_angle_.to_turns(),
            point_.distance_code.to_meters(),
            point_.intensity_code.bits
            // heap_alloc_elapsed_us.count()
            // points[0].distance_code.to_meters(),
            // points[0].intensity
            // hal::PE<1>().read().to_bool
        );
        // report_timer.invoke_if([&]{
        // });
    }


}

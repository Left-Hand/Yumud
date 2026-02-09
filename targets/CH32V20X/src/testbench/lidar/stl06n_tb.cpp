#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/utils/zero.hpp"
#include "core/async/timer.hpp"

// #include "core/mem/o1heap/o1heap_alloc.hpp"
#include "core/mem/o1heap/o1heap_alloc.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/utils/split_iter.hpp"
#include "core/string/utils/line_input_sinker.hpp"
#include "core/utils/iter/foreach.hpp"
#include "core/clock/time.hpp"
#include "core/string/utils/simularity/ngram.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Proximeter/STL06N/stl06n.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"

#include <ranges>
#include <unordered_map>
#include <map>


using namespace ymd;
using namespace ymd::drivers;



using LidarEvent = stl06n::Event;
using stl06n::PackedLidarPoint;

struct PackedCluster{
    std::array<PackedLidarPoint, 12> points;
    Angular<uq32> start_angle;
    Angular<uq32> stop_angle;
};


static constexpr auto LCD_WIDTH = 320u;
static constexpr auto LCD_HEIGHT = 170u;


void stl06n_main(){

    #if defined(CH32V20X)
    auto & DEBUG_UART = hal::usart2;
    DEBUG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(6000000),
        .tx_strategy = CommStrategy::Blocking
    });
    #elif defined(CH32V30X)
    auto & DEBUG_UART = hal::usart2;
    DEBUG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(6000000),
        .tx_strategy = CommStrategy::Blocking
    });
    #endif


    DEBUGGER.retarget(&DEBUG_UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(3);
    DEBUGGER.force_sync(EN);
    // DEBUGGER.no_fieldname(EN);
    DEBUGGER.no_fieldname(DISEN);


    auto watch_pin_ = hal::PA<11>();
    watch_pin_.set_mode(hal::GpioMode::OutPP);
    watch_pin_.set_low();

    volatile size_t lidar_sector_count_ = 0;
    volatile size_t lidar_ev_count_ = 0;
    volatile size_t lidar_crc_err_count_ = 0;


    Microseconds lidar_clone_elapsed_us_ = 0us;
    Angular<uq32> last_start_angle_ = Zero;
    Angular<uq32> last_stop_angle_ = Zero;
    // std::vector<PackedCluster> packed_clusters_;
    // packed_clusters_.reserve(64);

    static constexpr size_t POOL_SIZE = 4096 * 2;
    // static constexpr size_t POOL_SIZE = 6000;
    auto resource = std::make_unique<uint8_t[]>(POOL_SIZE);

    using Allocator = mem::o1heap::O1HeapAllocator<std::pair<const size_t, PackedCluster>>;
    auto o1heap_allocator = Allocator::try_from_buf(std::span(resource.get(), POOL_SIZE)).unwrap();

    std::map<
        size_t, 
        PackedCluster, 
        std::less<size_t>,
        Allocator
    > packed_clusters_(o1heap_allocator);
    // std::map<size_t, PackedCluster> packed_clusters_;

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

            // auto && view = make_std_range(sector.packed_points.iter())
            //     | std::views::take(2);

            const auto start_angle = sector.start_angle_code.to_angle();
            const auto stop_angle = sector.stop_angle_code.to_angle();
            if(last_start_angle_ > start_angle){
                lidar_sector_count_ = 0;
                // point_ = sector.packed_points[0];
            }else{
                lidar_sector_count_++;
            }

            auto & packed_points = packed_clusters_[size_t(lidar_sector_count_)];
            auto & points = packed_points.points;

            sector.packed_points.clone_to(std::span(points));
            packed_points.start_angle = start_angle;
            packed_points.stop_angle = stop_angle;

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
    auto & stl06n_uart_ = hal::usart1;
    stl06n_uart_.init({
        .remap = hal::USART1_REMAP_PA9_PA10,
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


    repl::ReplServer repl_server = {
        &DEBUG_UART, &DEBUG_UART
    };

    repl_server.enable_echo(EN);

    auto led = hal::PC<13>();
    led.outpp();


    auto add2 = [&](const iq24 a, const int8_t b) -> iq16 {return a + b;};

    auto repl_list =
        script::make_list( "list",
            script::make_function("rst", [](){sys::reset();}),
            script::make_function("outen", [&](){repl_server.enable_echo(EN);}),
            script::make_function("outdis", [&](){repl_server.enable_echo(DISEN);}),
            script::make_function("led", [&](const bool on){led.write(on ? HIGH : LOW);}),
            script::make_function("add2", add2),
            script::make_function("add", [&](const uint8_t a, const uint8_t b){return a + b;}),
            script::make_function("add3", [&](const float a, const float b){return a + b;}),
            script::make_function("ssm", [&](const StringView a, const StringView b){return str::ngram_similarity(a, b);}),
            script::make_function("test", [&](const StringView a){return strconv2::FstrDump::parse(a);}),

            script::make_list( "alct",
                script::make_function("now", [&](){return o1heap_allocator.diagnostics().allocated;}),
                script::make_function("peak", [&](){return o1heap_allocator.diagnostics().peak_allocated;})
            )
    );

    auto poll_main = [&]{
        
        // const auto heap_alloc_elapsed_us = measure_total_elapsed_us([&]{
        //     auto arena = std::make_unique<uint8_t[]>(128 * 64);
        // });

        // const auto & point = packed_clusters_[0].points[0];

        [[maybe_unused]] auto headed_points = packed_clusters_ 
            | std::views::values                    // Extract just the values (PackedCluster objects)
            | std::views::transform([](const PackedCluster & cluster) -> PackedLidarPoint { 
                // return cluster.points[0];
                //find min

                return *std::min_element(cluster.points.begin(), cluster.points.end(), 
                [](const PackedLidarPoint & a, const PackedLidarPoint & b){ return a.distance_code.bits < b.distance_code.bits; });
            })
            // | std::views::filter([i = 0](const auto&) mutable { 
            //     return (i++) % 4 == 0; 
            // })
            ;

        [[maybe_unused]] const auto & diagnostics = o1heap_allocator.diagnostics();

        std::array<iq16, 12> arr;
        static constexpr auto step = iq16(1.0 / 24.0);
        iq16 x = 0;
        const auto secs = clock::seconds();
        for(size_t i = 0; i < arr.size(); i++){
            arr[i] = math::sinpu(x + secs);
            x += step;
        }

        // poll_input_sinker();
        // repl_server.invoke(repl_list);

        if(false)DEBUG_PRINTLN(
            repl_server.line_sinker_.now_line(),
            // arr
            // arr
            // StringSplitIter("H \rE \rL L O", '\r'),

            0
            // headed_points
            // clock::millis().count(),
            // static_cast<uint8_t>(stl06n_parser_.fsm_state_),
            // static_cast<size_t>(stl06n_parser_.bytes_count_)
            // lidar_clone_elapsed_us_.count(),
            // last_start_angle_.to_turns(),
            // last_stop_angle_.to_turns(),
            // headed_points
            // std::size(packed_clusters_),
            // diagnostics.allocated,
            // diagnostics.capacity,
            // diagnostics.peak_allocated
            // point.distance_code.to_meters(),
            // point.intensity_code.bits,
            // std::size(headed_points),
            // std::size(packed_clusters_),
            // 0
            // heap_alloc_elapsed_us.count()
            // points[0].distance_code.to_meters(),
            // points[0].intensity
            // hal::PE<1>().read().to_bool
        );
        // report_timer.invoke_if([&]{
        // });
    };

    clock::delay(100ms);
    [[maybe_unused]] static auto report_timer = async::RepeatTimer::from_duration(8ms);

    while(true){
        repl_server.invoke(repl_list);
        report_timer.invoke_if([&]{
            poll_main();
        });
    }
}

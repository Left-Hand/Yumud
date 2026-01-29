#include <ranges>
#include <unordered_map>
#include <map>

#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/async/timer.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/utils/zero.hpp"
#include "core/utils/default.hpp"
#include "core/utils/iter/foreach.hpp"
#include "core/string/conv/strconv2.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/utils/split_iter.hpp"
#include "core/string/utils/line_input_sinker.hpp"
#include "core/string/utils/simularity/ngram.hpp"
#include "core/mem/o1heap/o1heap_alloc.hpp"

#include "algebra/shapes/prelude.hpp"

#include "primitive/arithmetic/rescaler.hpp"
#include "primitive/image/painter/painter.hpp"
#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"
#include "primitive/colors/rgb/rgb.hpp"
#include "primitive/colors/color/color.hpp"

#include "hal/gpio/gpio_port.hpp"
// #include "hal/timer/timer.hpp"
#include "hal/timer/hw_singleton.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/uart/uartsw.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"



#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"

#include "drivers/Proximeter/STL06N/stl06n.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"


#include "draw_iters.hpp"

using namespace ymd;
using namespace ymd::drivers;


static constexpr auto LCD_WIDTH = 320u;
static constexpr auto LCD_HEIGHT = 170u;


using LidarEvent = stl06n::Event;
using stl06n::PackedLidarPoint;

struct PackedCluster{
    std::array<PackedLidarPoint, 12> points;
    Angular<uq32> start_angle;
    Angular<uq32> stop_angle;
};


void lidar_visualize_main(){

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

    static constexpr size_t POOL_SIZE = 4096 * 2;
    auto resource = std::make_unique<uint8_t[]>(POOL_SIZE);

    using Allocator = mem::o1heap::O1HeapAllocator<std::pair<const size_t, PackedCluster>>;
    auto o1heap_allocator = Allocator::try_from_buf(std::span(resource.get(), POOL_SIZE)).unwrap();

    std::map<
        size_t, 
        PackedCluster, 
        std::less<size_t>,
        Allocator
    > packed_clusters_(o1heap_allocator);

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

            // auto && view = make_std_range(sector.packed_cluster.iter())
            //     | std::views::take(2);

            const auto start_angle = sector.start_angle_code.to_angle();
            const auto stop_angle = sector.stop_angle_code.to_angle();
            if(last_start_angle_ > start_angle){
                lidar_sector_count_ = 0;
                // point_ = sector.packed_cluster[0];
            }else{
                lidar_sector_count_++;
            }

            auto & packed_cluster = packed_clusters_[size_t(lidar_sector_count_)];
            auto & points = packed_cluster.points;

            sector.packed_cluster.clone_to(std::span(points));
            packed_cluster.start_angle = start_angle;
            packed_cluster.stop_angle = stop_angle;

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



    repl::ReplServer repl_server = {
        &DEBUG_UART, &DEBUG_UART
    };

    repl_server.set_outen(EN);

    auto led = hal::PC<13>();
    led.outpp();


    #ifdef CH32V30X
    auto & spi = hal::spi2;
    spi.init({
        .remap = hal::SPI2_REMAP_PB13_PB14_PB15_PB12,
        .baudrate = hal::NearestFreq(72_MHz)
    });

    #if 1
    auto & timer = hal::timer3;

    timer.init({
        .remap = hal::TIM3_REMAP_A6_A7_B0_B1,
        .count_freq = hal::NearestFreq(2_KHz),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().alter_to_pins({
        hal::TimerChannelSelection::CH3,
        hal::TimerChannelSelection::CH4,
    }).unwrap();

    auto & oc4 = timer.oc<4>();
    oc4.init(Default);
    oc4.set_dutycycle(0.3_uq16);
    timer.start();
    #endif

    auto lcd_blk_pin = hal::PD<0>();
    lcd_blk_pin.outpp(HIGH);

    auto lcd_dc_pin = hal::PD<7>();
    auto lcd_nrst_pin = hal::PB<7>();
    auto lcd_cs_pin = hal::PB<4>();

    #else
    auto & spi = hal::spi1;
    spi.init({
        .remap = hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
        .baudrate = hal::NearestFreq(72_MHz)
    });

    auto lcd_blk_pin = hal::PD<0>();
    lcd_blk_pin.outpp(HIGH);

    auto lcd_dc_pin = hal::PD<7>();
    auto lcd_nrst_pin = hal::PB<7>();
    auto lcd_cs_pin = hal::PD<4>();
    #endif

    lcd_nrst_pin.outpp();
    lcd_nrst_pin.set_low();
    clock::delay(10ms);
    lcd_nrst_pin.set_high();



    const auto spi_rank = spi.allocate_cs_pin(&lcd_cs_pin).unwrap();

    drivers::ST7789 tft{
        drivers::ST7789_Transport{&spi, spi_rank, &lcd_dc_pin, &lcd_nrst_pin}, 
        {LCD_WIDTH, LCD_HEIGHT}
    };

    tft.init(drivers::st7789_preset::_320X170{}).examine();


    std::array<RGB565, LCD_WIDTH> line_buffer;

    [[maybe_unused]] auto plot_rgb = [&](
        const std::span<const RGB565> src, 
        const ScanLine line
    ){
        tft.put_texture(
            line.bounding_box(),
            src.data()
        ).examine();
    };

    [[maybe_unused]] auto ch_font = MonoFont7x7{};
    [[maybe_unused]] auto en_font = MonoFont8x5{};
    [[maybe_unused]] auto en_font2 = MonoFont16x8{};

    static constexpr size_t IMAGE_WIDTH = 128;
    auto image = make_image<RGB565>(Vec2u{IMAGE_WIDTH, IMAGE_WIDTH});
    image.fill(color_cast<RGB565>(ColorEnum::LIGHT_YELLOW));

    uint16_t zoom = 50;

    auto repl_list =
        script::make_list( "list",
            script::make_function("rst", [](){sys::reset();}),
            script::make_function("outen", [&](){repl_server.set_outen(EN);}),
            script::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),
            script::make_function("led", [&](const bool on){led.write(on ? HIGH : LOW);}),
            script::make_function("add", [&](const uint8_t a, const uint8_t b){return a + b;}),
            script::make_function("add3", [&](const float a, const float b){return a + b;}),
            script::make_function("ssm", [&](const StringView a, const StringView b){return str::ngram_similarity(a, b);}),
            script::make_function("test", [&](const StringView a){return strconv2::FstrDump::parse(a);}),
            script::make_mut_property("zoom", &zoom),

            script::make_list( "alct",
                script::make_function("now", [&](){return o1heap_allocator.diagnostics().allocated;}),
                script::make_function("peak", [&](){return o1heap_allocator.diagnostics().peak_allocated;})
            )
    );


    clock::delay(100ms);
    [[maybe_unused]] static auto report_timer = async::RepeatTimer::from_duration(8ms);

    auto plot_clusters = [&](auto && range){
        for(const PackedCluster & cluster : range){
            if(cluster.start_angle < cluster.start_angle.DEG_90) continue;
            const PackedLidarPoint min_point = *std::min_element(cluster.points.begin(), cluster.points.end(), 
                [](const PackedLidarPoint & a, const PackedLidarPoint & b){ return a.distance_code.bits < b.distance_code.bits; });

            const auto angle = cluster.start_angle;
            const auto [s, c] = angle.sincos();
            const int16_t dx = int16_t(min_point.distance_code.to_meters() * iq16(c) * zoom);
            const int16_t dy = int16_t(min_point.distance_code.to_meters() * iq16(s) * zoom);

            const uint16_t x = static_cast<uint16_t>(CLAMP(-dx + (IMAGE_WIDTH / 2), 0, (IMAGE_WIDTH - 1)));
            const uint16_t y = static_cast<uint16_t>(CLAMP(dy + (IMAGE_WIDTH / 2), 0, (IMAGE_WIDTH - 1)));

            const auto color = color_cast<RGB565>(RGB<iq16>::from_hsv(
                min_point.intensity_code.bits * iq16(1.0 / 255.0), 
                iq16(1.0), iq16(1.0))
            );
            image.put_pixel(Vec2u16{static_cast<uint16_t>(x     ), static_cast<uint16_t>    (y)}, color);
            image.put_pixel(Vec2u16{static_cast<uint16_t>(x     ), static_cast<uint16_t>    (y + 1)}, color);
            image.put_pixel(Vec2u16{static_cast<uint16_t>(x+1   ), static_cast<uint16_t>    (y)}, color);
            image.put_pixel(Vec2u16{static_cast<uint16_t>(x+1   ), static_cast<uint16_t>    (y+1)}, color);
        }
    };

    while(true){
        repl_server.invoke(repl_list);
        const auto now_secs = clock::seconds();

        plot_clusters(packed_clusters_ | std::views::values);
        auto && shape = Sprite<RGB565>{.image = image.copy(), .position = Vec2u{10, 10}};

        auto shape_bb = shape.bounding_box();
        auto render_iter = RenderIterator<Sprite<RGB565>>(std::move(shape));

        Microseconds upload_elapsed_us = 0us;
        Microseconds render_elapsed_us = 0us;
        Microseconds clear_elapsed_us = 0us;
        const auto total_us = measure_total_elapsed_us([&]{
            for(uint16_t i = 0; i < LCD_HEIGHT; i++){
                
                auto line_buffer_span = LineBufferSpan<RGB565>(std::span(line_buffer), i);
                auto guard = make_scope_guard([&]{
                    clear_elapsed_us += measure_total_elapsed_us([&]{
                        line_buffer_span.fill(color_cast<RGB565>(ColorEnum::GRAY)).examine();
                    });
                });

                render_elapsed_us += measure_total_elapsed_us([&]{
                    if(not shape_bb.contains_y(i)) return;

                    if(static_cast<uint16_t>(i) == shape_bb.y()){
                        render_iter = make_draw_dispatch_iterator(shape);
                    }

                    if(render_iter.has_next()){
                        for(uint16_t j = 0; j < 1; j++){

                            [[maybe_unused]] const auto color = color_cast<RGB565>(
                                RGB<iq16>::from_hsv(
                                    math::frac(now_secs/2),
                                    iq16(1),
                                    iq16(1)
                                )
                            );

                            // render_iter.draw_filled(line_buffer_span, color).examine();
                            render_iter.draw_texture(line_buffer_span).examine();
                        }

                        render_iter.seek_next();
                    }{

                    }
                });

                upload_elapsed_us += measure_total_elapsed_us([&]{
                    plot_rgb(line_buffer_span.view(), line_buffer_span.to_scanline());
                });
            }
        });

        image.fill(color_cast<RGB565>(ColorEnum::LIGHT_YELLOW));

        if(false)DEBUG_PRINTLN(
            now_secs,
            static_cast<uint32_t>(render_elapsed_us.count()),
            // static_cast<uint32_t>(upload_elapsed_us.count())
            shape_bb
            // shape.center
            // ,shape.points
            // ,render_iter.is_mid_at_right()
            // clear_elapsed_us.count(), 
            // upload_elapsed_us.count(), 
            // total_us.count(),
            // shape_bb
            
            // clock::micros().count()

            // render_iter
            // shape_bb
        );

    }


    __builtin_trap();
}



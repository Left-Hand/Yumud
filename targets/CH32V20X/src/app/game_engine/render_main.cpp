#include "draw_iters.hpp"



#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/utils/nth.hpp"
#include "core/utils/stdrange.hpp"
#include "core/utils/data_iter.hpp"
#include "core/string/conv/strconv2.hpp"

#include "primitive/arithmetic/rescaler.hpp"
#include "primitive/image/painter/painter.hpp"
#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"
#include "primitive/colors/rgb/rgb.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"



#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"


#include "drivers/Display/Polychrome/ST7789/st7789.hpp"


using namespace ymd;


static constexpr auto LCD_WIDTH = 320u;
static constexpr auto LCD_HEIGHT = 170u;


void render_main(){
    auto init_debugger = []{

        hal::usart2.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(6000000),
            .tx_strategy = CommStrategy::Blocking
        });

        DEBUGGER.retarget(&hal::usart2);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        // DEBUGGER.no_brackets(EN);
    };

    init_debugger();

    #ifdef CH32V30X
    auto & spi = hal::spi2;
    spi.init({
        .remap = hal::SPI2_REMAP_PB13_PB14_PB15_PB12,
        .baudrate = hal::NearestFreq(72_MHz)
    });


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
    // tft.fill(color_cast<RGB565>(ColorEnum::BLACK)).examine();


    std::array<RGB565, LCD_WIDTH> line_buffer;

    [[maybe_unused]] auto plot_rgb = [&](
        const std::span<const RGB565> src, 
        const ScanLine line
    ){
        // DEBUG_PRINTLN(line.bounding_box());
        tft.put_texture(
            line.bounding_box(),
            src.data()
        ).examine();
    };

    [[maybe_unused]] auto ch_font = MonoFont7x7{};
    [[maybe_unused]] auto en_font = MonoFont8x5{};
    [[maybe_unused]] auto en_font2 = MonoFont16x8{};

    auto image = make_image<RGB565>(math::Vec2u{32, 32});
    // image.fill(RGB565::black);
    // image.at({0,1}) = color_cast<RGB565>(ColorEnum::BLUE);
    // image.at({10,11}) = color_cast<RGB565>(ColorEnum::BLUE);
    image.fill(color_cast<RGB565>(ColorEnum::PINK));
    while(true){
        const auto now_secs = clock::seconds();
        // const auto dest_angle = Angular<iq16>::from_turns(now_secs * 0.3_r);
        const auto dest_angle = Angular<iq16>::from_turns(now_secs * 0.3_r);
        // [[maybe_unused]] const auto [s,c] = math::sincospu(now_secs * 0.3_r);
        [[maybe_unused]] const auto [s, c] = dest_angle.sincos();
        [[maybe_unused]] const auto [shape_x, shape_y] = std::make_tuple(
            uint16_t(30 + 20 * iq16(c)), uint16_t(30 + 20 * iq16(s)));

        [[maybe_unused]] const auto samples = [&]{
            static constexpr auto LEN = 20;
            std::array<iq16, LEN> ret;
            for(size_t i = 0; i < LEN; i++){
                // ret[i] = 0.8_iq16 * sin(7 * now_secs + i * 0.15_r);
                ret[i] = 0.8_iq16 * math::sinpu(now_secs + i * 0.1_r);
            }
            return ret;
        } ();

        [[maybe_unused]] const auto factory = DemoShapeFactory{
            .now_secs = now_secs,
            .tft_bounding_box = tft.bounding_box(),
        };
        
        
        // const auto shape = factory.make_full_screen_rounded_rect();
        // const auto shape = factory.make_circle2();
        // const auto shape = factory.make_horizon_spectrum(samples);
        // const auto shape = factory.make_annular_sector();
        // const auto shape = factory.make_grid_map(8, 10);
        auto && shape = Sprite<RGB565>{.image = image.copy(), .position = math::Vec2u{shape_x, shape_y}};
        // const auto shape = factory.make_triangle2(dest_angle);

        // const auto shape = factory.make_horizon_oval2(Rect2<int16_t>::from_center_and_halfsize(
        //     {static_cast<int16_t>(160 + 160 * iq16(c)), 70},
        //     {60, 11}
        // ));

        // const auto shape = factory.make_line_text(en_font);
        // const auto shape = factory.make_segment2();
        // const auto shape = factory.make_rounded_rect2_moving();

        // using Shape = decltype(shape);
        auto shape_bb = shape.bounding_box();
        // auto render_iter = make_draw_dispatch_iterator(shape);
        auto render_iter = RenderIterator<Sprite<RGB565>>(std::move(shape));


        // PANIC{render_iter};

        Microseconds upload_elapsed_us = 0us;
        Microseconds render_elapsed_us = 0us;
        Microseconds clear_elapsed_us = 0us;
        const auto total_us = measure_total_elapsed_us([&]{
            for(uint16_t i = 0; i < LCD_HEIGHT; i++){
                
                auto line_buffer_span = LineBufferSpan<RGB565>(std::span(line_buffer), i);
                auto guard = make_scope_guard([&]{
                    clear_elapsed_us += measure_total_elapsed_us([&]{
                        line_buffer_span.fill(color_cast<RGB565>(ColorEnum::BLACK)).examine();
                    });
                });

                render_elapsed_us += measure_total_elapsed_us([&]{
                    if(not shape_bb.contains_y(i)) return;

                    if(static_cast<uint16_t>(i) == shape_bb.y()){
                        render_iter = make_draw_dispatch_iterator(shape);
                    }

                    if(render_iter.has_next()){
                        for(uint16_t j = 0; j < 1; j++){

                            // static constexpr auto color = color_cast<RGB565>(ColorEnum::PINK);
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


        DEBUG_PRINTLN(
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

};

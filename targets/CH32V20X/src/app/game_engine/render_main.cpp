#include "draw_iters.hpp"

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
        .baudrate = hal::NearestFreq(144_MHz)
    });


    auto lcd_blk = hal::PD<0>();
    lcd_blk.outpp(HIGH);

    auto lcd_dc = hal::PD<7>();
    auto lcd_nrst = hal::PB<7>();
    auto lcd_cs = hal::PD<4>();

    #else
    auto & spi = hal::spi1;
    spi.init({
        .remap = hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
        .baudrate = hal::NearestFreq(144_MHz)
    });

    auto lcd_blk = hal::PD<0>();
    lcd_blk.outpp(HIGH);

    auto lcd_dc = hal::PD<7>();
    auto lcd_nrst = hal::PB<7>();
    auto lcd_cs = hal::PD<4>();

    
    #endif


    const auto spi_rank = spi.allocate_cs_pin(&lcd_cs).unwrap();

    drivers::ST7789 tft{
        drivers::ST7789_Transport{&spi, spi_rank, &lcd_dc, &lcd_nrst}, 
        {LCD_WIDTH, LCD_HEIGHT}
    };

    tft.init(drivers::st7789_preset::_320X170{}).examine();

    std::array<RGB565, LCD_WIDTH> render_buffer;

    [[maybe_unused]] auto plot_rgb = [&](
        const std::span<RGB565> & src, 
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

    while(true){


        const auto now_secs = clock::seconds();
        // const auto dest_angle = Angular<iq16>::from_turns(now_secs * 0.3_r);
        const auto dest_angle = Angular<iq16>::from_turns(now_secs * 0.1_r);
        // [[maybe_unused]] const auto [s,c] = math::sincospu(now_secs * 0.3_r);
        [[maybe_unused]] const auto [s, c] = dest_angle.sincos();
        [[maybe_unused]] const auto [shape_x, shape_y] = std::make_tuple(
            uint16_t(30 + 20 * c), uint16_t(30 + 20 * s));

        [[maybe_unused]] const auto samples = [&]{
            static constexpr auto LEN = 20;
            std::array<iq16, LEN> ret;
            for(size_t i = 0; i < LEN; i++){
                // ret[i] = 0.8_iq16 * sin(7 * now_secs + i * 0.15_r);
                ret[i] = 0.8_iq16 * math::sinpu(now_secs + i * 0.1_r);
            }
            return ret;
        } ();

        const auto factory = DemoShapeFactory{
            .now_secs = now_secs,
            .tft_bounding_box = tft.bounding_box(),
        };
        
        
        // const auto shape = factory.make_full_screen_rounded_rect();
        // const auto shape = factory.make_circle2();
        const auto shape = factory.make_horizon_spectrum(samples);

        // using Shape = decltype(shape);
        auto shape_bb = shape.bounding_box();
        auto render_iter = make_draw_dispatch_iterator(shape);
        
        // PANIC{render_iter};

        Microseconds upload_us = 0us;
        Microseconds render_us = 0us;
        Microseconds clear_us = 0us;
        const auto total_us = measure_total_elapsed_us([&]{
            for(size_t i = 0; i < LCD_HEIGHT; i++){
                
                auto line_span = LineBufferSpan<RGB565>(std::span(render_buffer), i);
                auto guard = make_scope_guard([&]{
                    clear_us += measure_total_elapsed_us([&]{
                        line_span.fill(color_cast<RGB565>(ColorEnum::BLACK)).examine();
                    });
                });

                render_us += measure_total_elapsed_us([&]{
                    if(not shape_bb.has_y(i)) return;

                    if(i == shape_bb.y()){
                        render_iter = make_draw_dispatch_iterator(shape);
                    }

                    if(render_iter.has_next()){
                        for(size_t j = 0; j < 1; j++){

                            static constexpr auto color = color_cast<RGB565>(ColorEnum::PINK);

                            render_iter.draw_filled(line_span, color).examine();
                        }

                        render_iter.forward();
                    }{

                    }
                });

                upload_us += measure_total_elapsed_us([&]{
                    plot_rgb(line_span.iter(), line_span.to_scanline());
                });
            }
        });

        
        if(0) DEBUG_PRINTLN(
            // shape.center
            // ,shape.points
            // ,render_iter.is_mid_at_right()
            // clear_us.count(), 
            // upload_us.count(), 
            // total_us.count(),
            // shape_bb
            
            // clock::micros().count()

            // render_iter
            // shape_bb
        );

    }

};

#if 0
[[maybe_unused]] static void static_test(){
    {
        static constexpr auto len = []{
            std::array<RGB565, LCD_WIDTH * LCD_HEIGHT> render_buffer;

            auto frame_span = FrameBufferSpan<RGB565>::from_ptr_and_size(
                render_buffer.data(), {LCD_WIDTH, LCD_HEIGHT}).unwrap();

            auto iter = frame_span.iter();

            return count_iter(iter);
        }();

        static_assert(len == LCD_HEIGHT);
    }
}

#endif
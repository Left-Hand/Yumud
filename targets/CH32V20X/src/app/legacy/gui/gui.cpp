#include "gui.hpp"

#include "core/debug/debug.hpp"
#include "core/system.hpp"
#include "core/clock/time.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "middlewares/nvcv2/shape/shape.hpp"
#include "image/font/instance.hpp"

#include "elements.hpp"

#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::nvcv2;


#define WHARE_OK while(true){DEBUGGER.println(millis());};

class RenderIntf{
public:
    virtual void render(PainterBase & painter) = 0;
};


class Icon:public RenderIntf{
protected:
    Rect2u rect_ = {0,0,30,30};
    String name_ = "nil";
public:
    Icon() = default;

    void render(PainterBase & painter) override{
        painter.set_color(ColorEnum::WHITE);
        painter.draw_filled_rect(rect_).examine();
        // painter.drawString(rect_ + Vec2u{0, -10}, name_);
    }
};

class Menu:public RenderIntf{
protected:
    using Items = std::vector<Icon *>;

    Items items_;

    int item_padding_ = 10;
    Vec2u item_org_ = {10,10};

    void draw_otherwides(PainterBase & painter){
        painter.set_color(ColorEnum::WHITE);
    }
public:
    Menu() = default;

    void render(PainterBase & painter) override{
        // auto item_org = item_org_;
        // for(auto it = items_.begin(); it != items_.end(); ++it){
        // for(auto item : items_){
            // painter.draw_filled_rect(item_org, it);
        // };

        // draw_otherwids(painter);

        
    }
};




void gui_main(){
    auto & led = hal::PC<15>();
    led.outpp();
    // while(true){
    //     led = true;
    //     clock::delay(200ms);
    //     led = false;
    //     clock::delay(200ms);
    // }

    DEBUGGER_INST.init({576000});
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    #ifdef CH32V30X
    auto & spi = spi2;
    auto & lcd_blk = hal::PC<7>();
    
    lcd_blk.outpp(HIGH);

    auto & lcd_cs = hal::PD<6>();
    auto & lcd_dc = hal::PD<7>();
    auto & dev_rst = hal::PB<7>();
    #else
    auto & spi = spi1;
    auto & lcd_blk = hal::PA<10>();
    auto & lcd_cs = hal::PA<15>();
    auto & lcd_dc = hal::PA<11>();
    auto & dev_rst = hal::PA<12>();
    
    
    lcd_blk.outpp(HIGH);
    #endif


    const auto spi_rank = spi.allocate_cs_pin(lcd_cs).unwrap();
    spi.init({hal::NearestFreq(144_MHz)});
    // spi.init(36_MHz, CommStrategy::Blocking, CommStrategy::None);

    // ST7789 tft({{spi, 0}, lcd_dc, dev_rst}, {240, 134});
    ST7789 tft({spi, spi_rank, lcd_dc, dev_rst}, {240, 135});

    drivers::init_lcd(tft, st7789_preset::_240X135).examine();

    // Painter<RGB565> painter = Painter<RGB565>();


    // painter.bindImage(tft);
    // painter.fill(ColorEnum::BLACK);

    // painter.setChFont(ymd::font7x7);
    // painter.setEnFont(ymd::font8x5);

    // Theme theme{
    //     .stroke_color =  {70,70,70},
    //     .bg_color =     {10,10,10},
    //     .text_color =   ColorEnum::PINK
    // };
    
    // Label label{theme};
    // label.text = "hello world";


    // Label label2{theme};
    // label2.text = "你好世界";
 
    // Slider slider{theme};

    // OptionButton opt{theme};
    
    // I2cSw       i2c{hal::PD<2>(), hal::PC<12>()};
    // i2c.init(125_KHz);
    
    // MT9V034 camera{i2c};
    // camera.init();
    // camera.setExposureValue(1200);

    // [[maybe_unused]] auto plot_gray = [&](const Image<Gray> & src, const Vec2u & pos){
    //     auto area = Rect2u(pos, src.size());
    //     tft.put_texture(area, src.get_data());
    // };

    // [[maybe_unused]] auto plot_bina = [&](const Image<Binary> & src, const Vec2u & pos){
    //     auto area = Rect2u(pos, src.size());
    //     tft.put_texture(area, src.get_data());
    // };

    [[maybe_unused]] auto plot_rgb = [&](const Image<RGB565> & src, const Vec2u & pos){
        auto area = Rect2<uint16_t>(pos, src.size());
        tft.put_texture(area, src.get_data()).examine();
    };

    Image<RGB565> img{{tft.size().x, 4u}};

    // Painter<RGB565> painter = {};
    // painter.bind_image(tft);
    // painter.set_color(ColorEnum::BLACK);
    // painter.draw_filled_rect(tft.size().to_rect()).examine();

    #if 0
    while(true){
        painter.bind_image(img);
        painter.set_color(HSV888{0, int(100 + 100 * math::sinpu(clock::seconds())), 255});
        painter.draw_pixel(Vec2u(0, 0));
        painter.draw_hollow_rect(Rect2u(20, 0, 20, 40)).examine();

        tft.put_texture(img.size().to_rect(), img.get_data()).examine();
        DEBUG_PRINTLN(clock::millis());
    }
    #endif
}

        // #ifdef DRAW_TB
        // painter.set_color(ColorEnum::WHITE);
        
        // painter.drawString({20,20 + 10 * sin(t)}, String(millis()));
        // painter.drawString({20,20}, String(millis()));

        // painter.set_color(ColorEnum::RED);
        // painter.draw_filled_rect({60,60 + 10 * sin(t),20,20});
        // Rect2u rect = {30,40,80,50};
        // painter.draw_filled_rect(rect);
        // painter.drawHollowRect(rect);
        // painter.drawPixel(rect.position);
        // painter.drawLine(rect.position, rect.get_end());
        // painter.set_color(ColorEnum::BLUE);
        // painter.drawHollowRect(rect);
        // painter.draw_filled_rect(rect);
        // painter.drawFilledCircle(rect.position, 15);

        // painter.drawString({0,0}, "hello");
        // painter.draw_filled_rect(rect);
        // logger.println(rect, tft.get_view().intersection(rect));

        // painter.println(millis());

        // // logger.println(millis());
        // #endif


        // // #define CAMERA_TB
        // #ifdef CAMERA_TB
        // auto sketch = make_image<Gray>(camera.size()/2);
        // auto img = Shape::x2(camera);
        // tft.put_texture(img.get_view(), img.get_data());
        // clock::delay(10ms);
        // #endif


        // // DEBUG_PRINTLN(millis(), micros(), t);
        // // usart2.println(millis(), micros(), t, nanos());
        // // clock::delay(1ms);
        // // clock::delay(180us);
        // // clock::delay(380us);
        // // clock::delay(1ms);
        // // if(millis() > 10000) Sys::Misc::reset();
        // // while(uart7.pending());
        // // clock::delay(500us);

        // #define GUI_TB
        // #ifdef GUI_TB

        // painter.set_color(ColorEnum::BLACK);
        // painter.draw_filled_rect(painter.getClipWindow());

        // // label.rect = Rect2u{15 + 10 * sin(4 * t),20,100,20};
        // // label2.rect = Rect2u{15,80 + 20 * sin(4 * t),100,20};
        
        // slider.rect = Rect2u{15,20,100,20};
        // opt.rect = Rect2u{15,40,100,20};

        // // label.render(painter);
        // // label2.render(painter);
        // slider.render(painter);
        // opt.render(painter);


        // // clock::delay(200ms);
        // // painter.fill(ColorEnum::RED);
        // // auto m = micros();
        // // painter.fill(ColorEnum::GREEN);
        // // usart2.println(1000000 / (micros() - m));
        // // usart2.println(millis(), usart2.available());
        // // usart2.println(micros() - m);
        // // painter.fill(ColorEnum::BLUE);


        // #endif

        // // #define DRAW_TB
        // #ifdef DRAW_TB
        // painter.set_color(ColorEnum::WHITE);
        // // painter.drawString({0,0}, "what");
        // painter.drawFilledCircle({20,20}, 17);
        // painter.drawFilledTriangle({80,80}, {100,110}, {70,100});
        // // painter.drawPolyline({{80,80}, {100,110}, {70,100}});
        // painter.drawLine({30,20}, {80,50});
        // painter.drawLine({40,40}, {10,50});
        // // painter.drawLine({20,20}, {90,210});
        // clock::delay(20ms);
        // tft.fill(ColorEnum::BLACK);
        // clock::delay(20ms);
        // tft.fill(ColorEnum::BLACK);
        // #endif
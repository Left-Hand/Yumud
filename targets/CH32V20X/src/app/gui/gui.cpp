#include "gui.hpp"

#include "sys/debug/debug.hpp"
#include "sys/core/system.hpp"
#include "sys/clock/time.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "nvcv2/shape/shape.hpp"
#include "image/font/instance.hpp"

#include "Renderer.hpp"
#include "elements.hpp"

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::nvcv2;
using namespace ymd::hal;

using Vector2i = Vector2_t<int>;

#define WHARE_OK while(true){DEBUGGER.println(millis());};

class RenderTrait{
public:
    virtual void render(PainterConcept & painter) = 0;
};


class Icon:public RenderTrait{
protected:
    Rect2i rect_ = {0,0,30,30};
    String name_ = "nil";
public:
    Icon() = default;

    void render(PainterConcept & painter) override{
        painter.setColor(ColorEnum::WHITE);
        painter.drawFilledRect(rect_);
        // painter.drawString(rect_ + Vector2i{0, -10}, name_);
    }
};

class Menu:public RenderTrait{
protected:
    using Items = std::vector<Icon *>;

    Items items_;

    int item_padding_ = 10;
    Vector2i item_org_ = {10,10};

    void draw_otherwides(PainterConcept & painter){
        painter.setColor(ColorEnum::WHITE);
    }
public:
    Menu() = default;

    void render(PainterConcept & painter) override{
        // auto item_org = item_org_;
        // for(auto it = items_.begin(); it != items_.end(); ++it){
        // for(auto item : items_){
            // painter.drawFilledRect(item_org, it);
        // };

        // draw_otherwids(painter);

        
    }
};




void gui_main(){
    auto & led = portC[15];
    led.outpp();
    // while(true){
    //     led = true;
    //     delay(200);
    //     led = false;
    //     delay(200);
    // }

    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    #ifdef CH32V30X
    auto & spi = spi2;
    auto & lcd_blk = portC[7];
    
    lcd_blk.outpp(HIGH);

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];
    #else
    auto & spi = spi1;
    auto & lcd_blk = portA[10];
    auto & lcd_cs = portA[15];
    auto & lcd_dc = portA[11];
    auto & dev_rst = portA[12];
    
    
    lcd_blk.outpp(HIGH);
    #endif

    


    spi.bindCsPin(lcd_cs, 0);
    spi.init(144_MHz, CommMethod::Blocking, CommMethod::None);
    // spi.init(36_MHz, CommMethod::Blocking, CommMethod::None);

    // ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 134});
    ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 135});

    {
        tftDisplayer.init();


        if(true ){
        // if(false){
            tftDisplayer.setFlipX(false);
            tftDisplayer.setFlipY(true);
            tftDisplayer.setSwapXY(true);
            tftDisplayer.setDisplayOffset({40, 52}); 
        }else{
            tftDisplayer.setFlipX(true);
            tftDisplayer.setFlipY(true);
            tftDisplayer.setSwapXY(false);
            tftDisplayer.setDisplayOffset({52, 40}); 
        }
        tftDisplayer.setFormatRGB(true);
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);
    }

    // Painter<RGB565> painter = Painter<RGB565>();


    // painter.bindImage(tftDisplayer);
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
    
    // I2cSw       i2c{portD[2], portC[12]};
    // i2c.init(125_KHz);
    
    // MT9V034 camera{i2c};
    // camera.init();
    // camera.setExposureValue(1200);

    [[maybe_unused]] auto plot_gray = [&](const Image<Grayscale> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.size());
        tftDisplayer.putTexture(area, src.get_data());
    };

    [[maybe_unused]] auto plot_bina = [&](const Image<Binary> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.size());
        tftDisplayer.putTexture(area, src.get_data());
    };

    [[maybe_unused]] auto plot_rgb = [&](const Image<RGB565> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.size());
        tftDisplayer.putTexture(area, src.get_data());
    };

    Image<RGB565> img{{tftDisplayer.rect().w, 4}};

    Renderer renderer = {};
    renderer.bind(tftDisplayer);
    renderer.setColor(ColorEnum::BLACK);
    renderer.drawRect(tftDisplayer.rect());

    while(true){
        renderer.bind(img);
        renderer.setColor(HSV888{0, int(100 + 100 * sin(time())), 255});
        renderer.drawPixel(Vector2i(0, 0));
        renderer.drawRect(Rect2i(20, 0, 20, 40));

        tftDisplayer.putTexture(img.rect(), img.get_data());
    }
}

        // #ifdef DRAW_TB
        // painter.setColor(ColorEnum::WHITE);
        
        // painter.drawString({20,20 + 10 * sin(t)}, String(millis()));
        // painter.drawString({20,20}, String(millis()));

        // painter.setColor(ColorEnum::RED);
        // painter.drawFilledRect({60,60 + 10 * sin(t),20,20});
        // Rect2i rect = {30,40,80,50};
        // painter.drawFilledRect(rect);
        // painter.drawHollowRect(rect);
        // painter.drawPixel(rect.position);
        // painter.drawLine(rect.position, rect.get_end());
        // painter.setColor(ColorEnum::BLUE);
        // painter.drawHollowRect(rect);
        // painter.drawFilledRect(rect);
        // painter.drawFilledCircle(rect.position, 15);

        // painter.drawString({0,0}, "hello");
        // painter.drawFilledRect(rect);
        // logger.println(rect, tftDisplayer.get_view().intersection(rect));

        // painter.println(millis());

        // // logger.println(millis());
        // #endif


        // // #define CAMERA_TB
        // #ifdef CAMERA_TB
        // auto sketch = make_image<Grayscale>(camera.size()/2);
        // auto img = Shape::x2(camera);
        // tftDisplayer.puttexture(img.get_view(), img.get_data());
        // delay(10);
        // #endif


        // // DEBUG_PRINTLN(millis(), micros(), t);
        // // uart2.println(millis(), micros(), t, nanos());
        // // delay(1);
        // // delayMicroseconds(180);
        // // delayMicroseconds(380);
        // // delay(1);
        // // if(millis() > 10000) Sys::Misc::reset();
        // // while(uart7.pending());
        // // delayMicroseconds(500);

        // #define GUI_TB
        // #ifdef GUI_TB

        // painter.setColor(ColorEnum::BLACK);
        // painter.drawFilledRect(painter.getClipWindow());

        // // label.rect = Rect2i{15 + 10 * sin(4 * t),20,100,20};
        // // label2.rect = Rect2i{15,80 + 20 * sin(4 * t),100,20};
        
        // slider.rect = Rect2i{15,20,100,20};
        // opt.rect = Rect2i{15,40,100,20};

        // // label.render(painter);
        // // label2.render(painter);
        // slider.render(painter);
        // opt.render(painter);


        // // delay(200);
        // // painter.fill(ColorEnum::RED);
        // // auto m = micros();
        // // painter.fill(ColorEnum::GREEN);
        // // uart2.println(1000000 / (micros() - m));
        // // uart2.println(millis(), uart2.available());
        // // uart2.println(micros() - m);
        // // painter.fill(ColorEnum::BLUE);


        // #endif

        // // #define DRAW_TB
        // #ifdef DRAW_TB
        // painter.setColor(ColorEnum::WHITE);
        // // painter.drawString({0,0}, "what");
        // painter.drawFilledCircle({20,20}, 17);
        // painter.drawFilledTriangle({80,80}, {100,110}, {70,100});
        // // painter.drawPolyline({{80,80}, {100,110}, {70,100}});
        // painter.drawLine({30,20}, {80,50});
        // painter.drawLine({40,40}, {10,50});
        // // painter.drawLine({20,20}, {90,210});
        // delay(20);
        // tftDisplayer.fill(ColorEnum::BLACK);
        // delay(20);
        // tftDisplayer.fill(ColorEnum::BLACK);
        // #endif
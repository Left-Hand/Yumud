#include "gui.hpp"

#include "sys/debug/debug_inc.h"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "nvcv2/shape/shape.hpp"
#include "image/font/instance.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#include "sys/math/int/int_t.hpp"

#include "elements.hpp"

#include "eyetrack/eyetrack.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

using namespace yumud::nvcv2;
using namespace etk;

using Vector2i = Vector2_t<int>;

#define WHARE_OK while(true){DEBUGGER.println(millis());};


using Sys::t;

using namespace yumud;
using namespace yumud::drivers;



void gui_main(){

    auto & logger = LOGGER_INST;

    #ifdef CH32V30X
    auto & spi = spi2;
    #else
    auto & spi = spi1;
    #endif

    logger.init(576000);
    auto & lcd_blk = portC[7];
    
    lcd_blk.outpp(1);

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];

    spi.bindCsPin(lcd_cs, 0);
    spi.init(144_MHz, CommMethod::Blocking, CommMethod::None);

    ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {135, 240});

    {//init tft
        tftDisplayer.init();
        // tftDisplayer.setDisplayOffset({51, 40}); 
        tftDisplayer.setDisplayOffset({53, 40}); 
        tftDisplayer.setFlipX(true);
        tftDisplayer.setFlipY(true);
        tftDisplayer.setSwapXY(false);
        tftDisplayer.setFormatRGB(true);
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);
    }

    Painter<RGB565> painter = Painter<RGB565>();
    painter.bindImage(tftDisplayer);
    // tftDisplayer.fill(RGB565::BLACK);

    painter.setChFont(yumud::font7x7);
    painter.setEnFont(yumud::font8x5);

    Theme theme{
        .stoke_color =  {70,70,70},
        .bg_color =     {10,10,10},
        .text_color =   ColorEnum::PINK
    };
    
    Label label{theme};
    label.text = "hello world";


    Label label2{theme};
    label2.text = "你好世界";
 
    Slider slider{theme};

    OptionButton opt{theme};
    
    I2cSw       i2c{portD[2], portC[12]};
    i2c.init(1000000);
    
    MT9V034 camera{i2c};
    camera.init();
    camera.setExposureValue(1200);

    PCA9685 pca{i2c};

    [[maybe_unused]] auto plot_gray = [&](const Image<Grayscale> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.get_size());
        tftDisplayer.puttexture(area, src.get_data());
    };

    [[maybe_unused]] auto plot_bina = [&](const Image<Binary> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.get_size());
        tftDisplayer.puttexture(area, src.get_data());
    };

    [[maybe_unused]] auto plot_rgb = [&](const Image<RGB565> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.get_size());
        tftDisplayer.puttexture(area, src.get_data());
    };


    auto eye_conf =         Eyes::Config{
            .l_center = Vector2i{62, 60},
            .r_center = Vector2i{62, 180},

            .eye_radius = 40,
            .iris_radius = 13,
            .pupil_radius = 7
    };

    Eyes eye{
        theme,
        eye_conf,
        {
            .yaw = pca[0],
            .pitch = pca[1],

            .upper_l = pca[2],
            .lower_l = pca[3],

            .upper_r = pca[4],
            .lower_r = pca[5]
        }
    };

    while(true){

        #ifdef DRAW_TB
        painter.setColor(RGB565::WHITE);
        
        // painter.drawString({20,20 + 10 * sin(t)}, String(millis()));
        // painter.drawString({20,20}, String(millis()));

        painter.setColor(RGB565::RED);
        // painter.drawFilledRect({60,60 + 10 * sin(t),20,20});
        Rect2i rect = {30,7,12,20};
        // painter.drawFilledRect(rect);
        // painter.drawHollowRect(rect);
        painter.drawPixel(rect.position);
        painter.drawLine(rect.position, rect.get_end());
        painter.setColor(ColorEnum::BLUE);
        // painter.drawHollowRect(rect);
        // painter.drawFilledRect(rect);
        painter.drawFilledCircle(rect.position, 5);

        // painter.drawString({0,0}, "hello");
        // painter.drawFilledRect(rect);
        logger.println(rect, tftDisplayer.get_view().intersection(rect));

        // logger.println(millis());
        #endif


        // #define CAMERA_TB
        #ifdef CAMERA_TB
        auto sketch = make_image<Grayscale>(camera.get_size()/2);
        auto img = Shape::x2(camera);
        tftDisplayer.puttexture(img.get_view(), img.get_data());
        delay(10);
        #endif

        // #define GUI_TB
        #ifdef GUI_TB
        label.rect = Rect2i{15 + 10 * sin(t),20,100,20};
        label2.rect = Rect2i{15,80 + 20 * sin(t),100,20};
        slider.rect = Rect2i{15,120,100,20};
        opt.rect = Rect2i{15,160,100,20};

        label.render(painter);
        label2.render(painter);
        slider.render(painter);
        opt.render(painter);


        delay(20);
        painter.fill(ColorEnum::BLACK);

        #endif

        // #define DRAW_TB
        #ifdef DRAW_TB
        painter.setColor(ColorEnum::WHITE);
        // painter.drawString({0,0}, "what");
        painter.drawFilledCircle({20,20}, 17);
        painter.drawFilledTriangle({80,80}, {100,110}, {70,100});
        // painter.drawPolyline({{80,80}, {100,110}, {70,100}});
        painter.drawLine({30,20}, {80,50});
        painter.drawLine({40,40}, {10,50});
        // painter.drawLine({20,20}, {90,210});
        delay(20);
        tftDisplayer.fill(ColorEnum::BLACK);
        delay(20);
        tftDisplayer.fill(ColorEnum::BLACK);
        #endif

        #define EYE_TB
        #ifdef EYE_TB
        eye.render(painter);

        eye.update(
            {Vector2::RIGHT.rotated(t * tau)}, 
            std::to_array({EyelidInfo{{0,0}}, EyelidInfo{{0,0}}})
        );

        delay(20);
        painter.fill(ColorEnum::BLACK);
        #endif
    
    }
}
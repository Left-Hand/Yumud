#include "gui.hpp"


#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

using Vector2i = Vector2_t<int>;

#define WHARE_OK while(true){DEBUGGER.println(millis());};


using Sys::t;

class CanvasItem{
public:
    struct Config{
        Rect2i rect;
    };  
    auto config() const{return config_;}
protected:
    Config config_;
public:
    // virtual void render(const Paint){}

    CanvasItem(const Config & _config): config_(_config){;}
};

class Label:public CanvasItem{
    
};

void gui_main(){

    auto & logger = uart2;
    auto & spi = spi2;
    logger.init(576000);
    auto & lcd_blk = portC[7];
    
    lcd_blk.outpp(1);

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];

    spi.bindCsPin(lcd_cs, 0);
    spi.init(144000000, CommMethod::Blocking, CommMethod::None);
    DisplayInterfaceSpi SpiInterfaceLcd{{spi, 0}, lcd_dc, dev_rst};

    ST7789 tftDisplayer(SpiInterfaceLcd, Vector2i(135, 240));

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

    // painter.setChFont(font7x7);
    painter.setEnFont(font8x5);
    while(true){
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
        painter.setColor(Colors::BLUE);
        // painter.drawHollowRect(rect);
        // painter.drawFilledRect(rect);
        painter.drawFilledCircle(rect.position, 5);

        // painter.drawString({0,0}, "hello");
        // painter.drawFilledRect(rect);
        logger.println(rect, tftDisplayer.get_view().intersection(rect));

        // logger.println(millis());
        delay(10);
        tftDisplayer.fill(RGB565::BLACK);
    }
}
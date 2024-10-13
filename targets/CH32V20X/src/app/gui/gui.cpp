#include "gui.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

using Vector2i = Vector2_t<int>;



void gui_main(){
    auto logger = uart2;
    logger.init(921600/2);

    auto & lcd_blk = portC[7];
    auto & spi = spi2;
    
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
        tftDisplayer.setDisplayOffset({51, 40}); 
        tftDisplayer.setFlipX(false);
        tftDisplayer.setFlipY(false);
        tftDisplayer.setSwapXY(false);
        tftDisplayer.setFormatRGB(true);
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);
    }

    Painter<RGB565> painter = Painter<RGB565>();
    painter.bindImage(tftDisplayer);
    // tftDisplayer.fill(RGB565::BLACK);

    painter.setChFont(font7x7);
    painter.setEnFont(font8x5);
    // StringView str = "Hello!!!";
    while(true){
        // painter.drawString({20,20}, String(millis()));
        painter.drawFilledRect({0,0,60,60}, RGB565::RED);
        logger.println(millis());
        // logger << millis();
        delay(1);
    }
}
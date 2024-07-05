#include "tb.h"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "nvcv2/pixels.hpp"
#include "nvcv2/shape.hpp"
#include "nvcv2/geometry.hpp"
#include "nvcv2/two_pass.hpp"
#include "nvcv2/flood_fill.hpp"

using namespace NVCV2;

void st77xx_tb(IOStream & logger, Spi & spi){
    auto & led = portC[14];
    auto & lcd_blk = portC[7];
    auto & light = portC[6];

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];

    led.outpp();
    lcd_blk.outpp(1);
    light.outpp(1);

    spi.bindCsPin(lcd_cs, 0);
    spi.init(144000000);

    SpiDrv SpiDrvLcd = SpiDrv(spi, 0);
    DisplayInterfaceSpi SpiInterfaceLcd{SpiDrvLcd, lcd_dc, dev_rst};

    ST7789 tftDisplayer(SpiInterfaceLcd, Vector2i(135, 240));

    {//init tft
        tftDisplayer.init();
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

    painter.setColor(RGB565::RED);
    painter.drawFilledCircle({30,30}, 8);



    I2cSw SccbSw(portD[2], portC[12]);
    SccbSw.init(10000);
    MT9V034 camera(SccbSw);

    [[maybe_unused]] auto plot_gray = [&](Image<Grayscale, Grayscale> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](Image<Binary, Binary> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    camera.init();
    // logger.println(camera.isChipValid());
    // {//try to init camera
    //     uint8_t camera_init_retry_times = 0;
    //     constexpr uint8_t camera_retry_timeout = 7;
    //     while(!camera.init()){
    //         camera_init_retry_times++;
    //         if(camera_init_retry_times >= camera_retry_timeout){
    //             // DEBUG_LOG("camera init failed");
    //             exit(0);
    //         }
    //     }
    // }
    // camera.setExposureValue(1200);

    // while(true){
    //     led = !led;
    //     tftDisplayer.fill(RGB565::BLACK);
    //     // tftDisplayer.fill(RGB565::GREEN);
    //     painter.drawFilledCircle({40,60 + 30 * sin(8 * t)}, 8);
    // }
    // mnist_tb();
    while(true){
        led = !led;


        // continue;
        auto img = Shape::x4(camera,2);
        // auto img = camera.clone();
        Pixels::inverse(img);
        auto bina = Pixels::binarization(img, 200);
        // Pixels::gamma(img, 0.1);
        plot_gray(img, img.get_window());
        // plot_bina(bina, bina.get_window() + Vector2i{0, img.size.y});
        // int dummy = 0;

        plot_bina(bina, bina.get_window() + Vector2i{0, img.size.y});

        Shape::FloodFill ff;
        auto map = ff.run(bina);

        Pixels::dyeing(map, map);
        plot_gray(map, map.get_window() + Vector2i{0, img.size.y * 2});

        // Shape::TwoPass tp(bina.w * bina.h);
        // map = tp.run(bina);

        // Pixels::dyeing(map, map);
        // plot_gray(map, map.get_window() + Vector2i{img.size.x, 0});
        // tp.run(img, bina, dummy);

        // tftDisplayer.puttexture_unsafe(img.get_window(), img.data.get());
        // plot_gray(camera, Rect2i{Vector2i{188/4, 0}, Vector2i{188/4, 60}});
        // painter.drawImage()
        // tftDisplayer.fill(RGB565::BLACK);
        // tftDisplayer.fill(RGB565::RED);
        // tftDisplayer.fill(RGB565::BLUE);
        // painter.drawString({0,0}, "Hello");
        // delay(100);
        // tftDisplayer.fill(RGB565::YELLOW);

        painter.setColor(RGB565::GREEN);
        painter.drawHollowRect({0,0,28,28});
        const auto & blobs = ff.blobs();
        painter.setColor(RGB565::RED);
        for(const auto & blob : blobs){
            painter.drawRoi(blob.rect);
        }

        // painter.drawHollowRect(Rect2i{12,12,60,30});

        // delay(10);
        // const auto & blob = blobs[0];
        // printf("%d, %d, %d, %d\r\n", blob.rect.x, blob.rect.w, blob.rect.h, blob.area);
        // printf("%d\r\n", blobs.size());
    }
}
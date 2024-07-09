#include "host.hpp"
#include "imgtrans/img_trans.hpp"

#include "nvcv2/pixels.hpp"
#include "nvcv2/shape.hpp"
#include "nvcv2/geometry.hpp"
#include "nvcv2/two_pass.hpp"
#include "nvcv2/flood_fill.hpp"
#include "nvcv2/mnist/mnist.hpp"

#include "interpolation/interpolation.hpp"

#include "drivers/LightSensor/TCS34725/tcs34725.hpp"



using namespace Interpolation;
using namespace NVCV2;

#ifdef CH32V30X


void EmbdHost::main(){

    auto & led = portC[14];
    auto & lcd_blk = portC[7];
    auto & spi = spi2;
    // auto & light = portC[6];
    
    led.outpp();
    lcd_blk.outpp(1);

    auto & light_pwm = timer8.oc(1);
    timer8.init(2000);
    light_pwm.init();
    light_pwm = 0.9;
    light_pwm.io().outpp(1);


    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];

    // light.outpp(1);

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
    tftDisplayer.fill(RGB565::BLACK);

    I2cSw i2c(portD[2], portC[12]);
    i2c.init(400000);
    MT9V034 camera(i2c);

    [[maybe_unused]] auto plot_gray = [&](Image<Grayscale, Grayscale> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](Image<Binary, Binary> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    camera.init();

    VL53L0X             vl(i2c);
    vl.init();
    vl.enableContMode();
    vl.startConv();

    TCS34725            tcs(i2c);
    tcs.init();
    tcs.startConv();

    uart7.init(1000000, CommMethod::Blocking);

    CH9141 ch9141{uart7, portC[1], portD[3]};
    ch9141.init();

    // Transmitter trans{ch9141};
    // Transmitter trans{logger};
    Transmitter trans{usbfs};
    Mnist mnist;

    auto img = Shape::x4(camera,2);
    auto bina = Pixels::binarization(img, 70);
    while(true){
        led = !led;
        // continue;
        img = Shape::x4(camera,2);
        bina = Pixels::binarization(img,  170);
        // trans.transmit(img, 0);
        // continue;
        // Pixels::inverse(img);
        // auto piece = Shape::x4(img,2);

        auto diff = img.space();
        Shape::convo_roberts_xy(diff, img);

        // auto diff_bina = make_bina_mirror(diff);
        // Pixels::binarization(diff_bina, diff, 40);
        // Pixels::or_with(bina, diff_bina);
        // Shape::morph_close(new_bina);
        // Pixels::copy(bina, new_bina);
        // Shape::erosion(bina, bina);
        // Pixels::and_with(bina, new_bina);
        // real_t dist = vl.getDistance();

        // logger.println(dist);
        // Pixels::gamma(img, 0.1);
        // plot_gray(img, img.get_window());
        // plot_bina(bina, bina.get_window() + Vector2i{0, img.size.y});
        // int dummy = 0;
        // logger.println("rect", bina.get_window() + Vector2i{0, img.size.y});
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
        // auto piece = img.clone({0, 0, 14, 28});


        // painter.drawHollowRect({0,0,28,28});


        const auto & blobs = ff.blobs();
        for(const auto & blob : blobs){
            if(int(blob) == 0 || int(blob.rect) == 0) continue;
            painter.setColor(RGB565::RED);
            painter.drawRoi(blob.rect);
            painter.setColor(RGB565::GREEN);
            painter.drawString(blob.rect.position, "2");
            // logger.println(blob.rect, int(blob.rect));
        }

        {
            painter.setColor(RGB565::YELLOW);
            painter.drawRoi({0,0,28,28});
        }

        {
            [[maybe_unused]] auto result = mnist.update(img, {0,0});
            // logger.println(result.token, result.confidence);
        }

        {
            vl.update();
            logger.println(vl.getDistance());
        }

        {
            // tcs.update();
            // logger.println(tcs.getCRGB());
        }
        // trans.transmit(img.clone(Rect2i(0,0,94/4,60/4)), 1);
        // trans.transmit(img, 1);
        // trans.transmit(bina, 0);
        // painter.drawString(Vector2i{0,230-60}, toString(vl.getDistance()));
        // logger.println(real_t(light_pwm));
        // painter.drawString(Vector2i{0,230-50}, toString(trans.compress_png(piece).size()));

        // delay(300);

        // painter.drawHollowRect(Rect2i{12,12,60,30});

        // delay(10);
        // const auto & blob = blobs[0];
        // f("%d, %d, %d, %d\r\n", blob.rect.x, blob.rect.w, blob.rect.h, blob.area);
        // printf("%d\r\n", blobs.size());
        // host.run();
    }
}

void EmbdHost::parse_command(const uint8_t id, const Command &cmd, const CanMsg &msg){


}

void EmbdHost::run() {
    CliAP::run();
    const real_t ang1 = 4 * t;
    const real_t ang2 = 3 * t;
    const real_t amp = 2;
    steppers.x.setTargetPosition(amp * sin(ang1));
    steppers.y.setTargetPosition(amp * cos(ang2));
    steppers.z.setTargetPosition(4 + sin(t));
    // logger.println("why");
    // steppers.y.setTargetCurrent(sign(sin(8 * t)));
    // logger.println(steppers.y.getSpeed());
    
    // steppers.z.setTargetPosition(ss());
    // logger.println(can1.getTxErrCnt(), can1.getRxErrCnt(), can1.getErrCode());
    // can.write(CanMsg{0x70});
}

#endif
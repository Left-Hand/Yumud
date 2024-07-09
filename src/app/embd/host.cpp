#include "host.hpp"
#include "imgtrans/img_trans.hpp"

#include "nvcv2/pixels.hpp"
#include "nvcv2/shape.hpp"
#include "nvcv2/geometry.hpp"
#include "nvcv2/two_pass.hpp"
#include "nvcv2/flood_fill.hpp"
#include "nvcv2/mnist/mnist.hpp"

#include "interpolation/interpolation.hpp"

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


    spi.bindCsPin(lcd_cs, 0);
    spi.init(144000000);
    usbfs.init();
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
    i2c.init(400000);


    [[maybe_unused]] auto plot_gray = [&](Image<Grayscale, Grayscale> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](Image<Binary, Binary> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_rgb = [&](Image<RGB565, RGB565> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };


    camera.init();
    camera.setExposureValue(800);


    vl.init();
    vl.enableContMode();
    vl.startConv();


    tcs.init();
    tcs.startConv();

    uart7.init(1000000, CommMethod::Blocking);


    ch9141.init();

    // Transmitter trans{ch9141};
    // Transmitter trans{logger};


    while(true){
        led = !led;
        // continue;
        auto img = Shape::x4(camera,2);
        plot_gray(img, img.get_window());

        auto diff = img.space();
        Shape::sobel_xy(diff, img);
        // plot_gray(diff, diff.get_window() + Vector2i{0, 2 * img.size.y});
        // auto diff_bina = Image<Binary, Binary>(img.get_size());
        auto diff_bina = make_bina_mirror(diff);
        Pixels::binarization(diff_bina, diff, diff_threshold);
        // plot_bina(diff_bina, diff_bina.get_window() + Vector2i{0, 3 * img.size.y});


        auto img_bina = Image<Binary, Binary>(img.get_size());
        // auto img_bina = make_bina_mirror(img);
        Pixels::binarization(img_bina, img, bina_threshold);
        Pixels::or_with(img_bina, diff_bina);

        // Shape::erosion(img_bina);
        plot_bina(img_bina, img.get_window() + Vector2i{0, img.size.y});
        
        // Pixels::binarization(img_bina, img,  bina_threshold);



        // trans.transmit(img, 0);
        // continue;
        // Pixels::inverse(img);
        // auto piece = Shape::x4(img,2);
        // auto diff2 = img.space();
        // Shape::sobel_xy(diff, img);
        
        // Shape::sobel_y(diff2, img);
        // Pixels::or_with(diff, diff2);
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

        // plot_bina(bina, bina.get_window() + Vector2i{0, img.size.y});
        // int dummy = 0;
        // logger.println("rect", bina.get_window() + Vector2i{0, img.size.y});
        // plot_bina(img_bina, img_bina.get_window() + Vector2i{0, img.size.y});

        Shape::FloodFill ff;
        auto map = ff.run(img_bina);
        Pixels::dyeing(map, map);
        plot_gray(map, diff.get_window() + Vector2i{0, 2 * img.size.y});

        // plot_gray(map, map.get_window() + Vector2i{0, img.size.y * 2});
        // plot_gray(diff, map.get_window() + Vector2i{0, img.size.y * 3});
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
        // for(const auto & blob : blobs){
        if(blobs.size()){
            const auto & blob = blobs[0];
            painter.setColor(RGB565::RED);
            painter.drawRoi(blob.rect);
            painter.setColor(RGB565::GREEN);
            Rect2i view = Rect2i::from_center(blob.rect.get_center(), Vector2i(14,14));
            painter.drawRoi(view);
            auto piece = img.clone(view);
            auto mask = img_bina.clone(view);
            Pixels::mask_with(piece, mask);
            Shape::gauss(piece);
            // Pixels::inverse(piece);
            Mnist mnist;
            auto result = mnist.update(piece);
            // logger.println(mnist.outputs);
            const auto & outputs = mnist.outputs;
            for(size_t i = 0; i < outputs.size(); i++){
                logger << outputs[i];
                if(i != outputs.size() - 1) logger << ',';
            };
            logger.println();
            // trans.transmit(piece, 1);
            // painter.drawString(blob.rect.position, "2");
            // logger.println(blob.rect, int(blob.rect), blob.index, blob.area);
        }

        // {
        //     painter.setColor(RGB565::YELLOW);
        //     painter.drawRoi({0,0,28,28});
        // }

        {
            
            // 
        }

        {
            vl.update();
        }

        {
            tcs.update();
            // RGB565 color = RGB888(tcs);
            
            // logger.println(tcs.getCRGB());
        }
        // trans.transmit(img.clone(Rect2i(0,0,94/4,60/4)), 1);
        // 
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


void EmbdHost::parse_command(const String & _command,const std::vector<String> & args){
    auto command = _command;
    command.toLowerCase();
    switch(hash_impl(command.c_str(), command.length())){
        case "exp"_ha:
            settle_method(camera.setExposureValue, args, int)
            break;
        case "bina"_ha:
            settle_value(bina_threshold, args)
            break;
        case "diff"_ha:
            settle_value(diff_threshold, args)
            break;
        default:
            CliAP::parse_command(_command, args);
            break;
    }
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
#include "host.hpp"
#include "imgtrans/img_trans.hpp"

#include "../nvcv2/pixels/pixels.hpp"
#include "../nvcv2/shape/shape.hpp"
#include "../nvcv2/geometry/geometry.hpp"
#include "../nvcv2/two_pass.hpp"
#include "../nvcv2/flood_fill.hpp"
#include "../nvcv2/mnist/mnist.hpp"

#include "interpolation/interpolation.hpp"

using namespace Interpolation;
using namespace NVCV2;

#ifdef CH32V30X


void EmbdHost::main(){
    delay(200);
    auto & led = portC[14];
    auto & lcd_blk = portC[7];
    auto & spi = spi2;
    
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
    DisplayInterfaceSpi SpiInterfaceLcd{{spi, 0}, lcd_dc, dev_rst};

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


    [[maybe_unused]] auto plot_gray = [&](Image<Grayscale> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](Image<Binary> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_rgb = [&](Image<RGB565> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };


    camera.init();
    camera.setExposureValue(800);


    vl.init();
    vl.enableContMode();
    vl.startConv();


    tcs.init();
    tcs.startConv();
    tcs.setIntegration(48);
    tcs.setGain(TCS34725::Gain::X60);
    ch9141.init();

    uart7.bindRxPostCb([&](){parseLine(uart7.readString());});
    // Transmitter trans{ch9141};
    // Transmitter trans{logger};
    // Transmitter trans{usbfs};


    while(true){
        led = !led;
        auto cw = camera.get_window()/2;
        auto img = camera.clone(cw);
        plot_gray(img, img.get_window());
        auto diff = img.space();
        Shape::sobel_xy(diff, img);
        auto diff_bina = make_bina_mirror(diff);
        Pixels::binarization(diff_bina, diff, diff_threshold);

        auto img_bina = Image<Binary>(img.get_size());
        Pixels::binarization(img_bina, img, bina_threshold);
        Pixels::or_with(img_bina, diff_bina);

        plot_bina(img_bina, img.get_window() + Vector2i{0, img.size.y});


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
        // plot_gray(map, diff.get_window() + Vector2i{0, 2 * img.size.y});

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
            // auto mask = img_bina.clone(view);
            // Pixels::mask_with(piece, mask);
            Shape::gauss(piece);
            // Pixels::inverse(piece);
            Mnist mnist;
            // auto result = mnist.update(piece);
            // logger.println(mnist.outputs);
            // const auto & outputs = mnist.outputs;
            // for(size_t i = 0; i < outputs.size(); i++){
            //     logger << outputs[i];
            //     if(i != outputs.size() - 1) logger << ',';
            // };
            // logger.println();
            trans.transmit(piece, 1);
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

        if(tcs.isIdle()){
            tcs.update();
            // RGB565 color = RGB888(tcs);
            real_t r,g,b,c;
            tcs.getCRGB(c,r,g,b);
            // logger.println(c,r,g,b, tcs.getId(), vl.getDistance());
            tcs.startConv();
        }
        // trans.transmit(img.clone(Rect2i(0,0,94/4,60/4)), 1);
        // 
        trans.transmit(img, 0);
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

void EmbdHost::parseCommand(const uint8_t id, const Command &cmd, const CanMsg &msg){


}


void EmbdHost::parseTokens(const String & _command,const std::vector<String> & args){
    auto command = _command;
    command.toLowerCase();
    switch(hash_impl(command.c_str(), command.length())){
        case "exp"_ha:
            settle_method(camera.setExposureValue, args, int)
        case "bina"_ha:
            settle_value(bina_threshold, args)
        case "diff"_ha:
            settle_value(diff_threshold, args)
        case "xyz"_ha:
            if(args.size() == 3){
                steppers.x.setTargetPosition(real_t(args[0]));
                steppers.y.setTargetPosition(real_t(args[1]));
                steppers.z.setTargetPosition(real_t(args[2]));
            }
            break;

        case "xyzt"_ha:
            if(args.size() == 3){
                steppers.x.setTargetTrapezoid(real_t(args[0]));
                steppers.y.setTargetTrapezoid(real_t(args[1]));
                steppers.z.setTargetTrapezoid(real_t(args[2]));
            }
            break;

        case "xy"_ha:
            if(args.size() == 2){
                steppers.x.setTargetPosition(real_t(args[0]));
                steppers.y.setTargetPosition(real_t(args[1]));
            }
            break;

        case "xyt"_ha:
            if(args.size() == 2){
                steppers.x.setTargetTrapezoid(real_t(args[0]));
                steppers.y.setTargetTrapezoid(real_t(args[1]));
            }
            break;

        case "dxy"_ha:
            if(args.size() == 2){
                steppers.x.setTargetSpeed(real_t(args[0]));
                steppers.y.setTargetSpeed(real_t(args[1]));
            }
            break;

        case "mt"_ha:
            if(args.size() == 1){
                steppers.y.mt(int(args[0]));
            }
            break;

        case "dxyz"_ha:
            if(args.size() == 3){
                steppers.x.setTargetSpeed(real_t(args[0]));
                steppers.y.setTargetSpeed(real_t(args[1]));
                steppers.z.setTargetSpeed(real_t(args[2]));
            }
            break;

        case "xp"_ha:
            settle_method(steppers.x.setTargetPosition, args, real_t)
        case "yp"_ha:
            settle_method(steppers.y.setTargetPosition, args, real_t);
        case "zp"_ha:
            settle_method(steppers.z.setTargetPosition, args, real_t);
        case "xc"_ha:
            settle_method(steppers.x.setTargetCurrent, args, real_t);
        case "yc"_ha:
            settle_method(steppers.y.setTargetCurrent, args, real_t);
        case "zc"_ha:
            settle_method(steppers.z.setTargetCurrent, args, real_t);
        case "xs"_ha:
            settle_method(steppers.x.setTargetSpeed, args, real_t);
        case "ys"_ha:
            settle_method(steppers.y.setTargetSpeed, args, real_t);
        case "zs"_ha:
            settle_method(steppers.z.setTargetSpeed, args, real_t);
        case "xt"_ha:
            settle_method(steppers.x.setTargetTrapezoid, args, real_t);
        case "yt"_ha:
            settle_method(steppers.y.setTargetTrapezoid, args, real_t);
        case "zt"_ha:
            settle_method(steppers.z.setTargetTrapezoid, args, real_t);
        case "xh"_ha:
            settle_method(steppers.x.locateRelatively, args, real_t);
        case "yh"_ha:
            settle_method(steppers.y.locateRelatively, args, real_t);
        case "zh"_ha:
            settle_method(steppers.z.locateRelatively, args, real_t);
        case "xm"_ha:
            settle_method(steppers.x.setCurrentClamp, args, real_t);
        case "ym"_ha:
            settle_method(steppers.y.setCurrentClamp, args, real_t);
        case "zm"_ha:
            settle_method(steppers.z.setCurrentClamp, args, real_t);
        case "cali"_ha:
            trigger_method(steppers.w.triggerCali);
        case "ld"_ha:
            trigger_method(steppers.w.loadArchive, true);
        case "rm"_ha:
            trigger_method(steppers.w.removeArchive, true);
        case "sv"_ha:
            trigger_method(steppers.w.saveArchive, true);
        case "rst"_ha:
            trigger_method(reset);
        case "nne"_ha:
            trigger_method(set_demo_method, ActMethod::NONE);
        case "hui"_ha:
            trigger_method(set_demo_method, ActMethod::HUI);
        case "lisa"_ha:
            trigger_method(set_demo_method, ActMethod::LISA);
        case "grab"_ha:
            trigger_method(set_demo_method, ActMethod::GRAB);
        case "inte"_ha:
            trigger_method(set_demo_method, ActMethod::INTER);
        case "rep"_ha:
            trigger_method(set_demo_method, ActMethod::REP);
        case "usbon"_ha:
            trigger_method(trans.enable, true);
        case "usboff"_ha:
            trigger_method(trans.enable, false);
        default:
            CliAP::parseTokens(_command, args);
            break;
    }
}

void EmbdHost::reset(){
    steppers.w.reset();
    Sys::Misc::reset();
}

void EmbdHost::cali(){
    steppers.w.triggerCali();
}


void EmbdHost::run() {
    readCan();
    act();
    // const real_t ang1 = 4 * t;
    // const real_t ang2 = 3 * t;
    // const real_t amp = 2;
    // steppers.x.setTargetPosition(amp * sin(ang1));
    // steppers.y.setTargetPosition(amp * cos(ang2));
    // steppers.z.setTargetPosition(4 + sin(t));
    // logger.println("why");
    // steppers.y.setTargetCurrent(sign(sin(8 * t)));
    // logger.println(steppers.y.getSpeed());
    
    // steppers.z.setTargetPosition(ss());
    // logger.println(can1.getTxErrCnt(), can1.getRxErrCnt(), can1.getErrCode());
    // can.write(CanMsg{0x70});

}

void EmbdHost::set_demo_method(const ActMethod new_method){
    act_method = new_method;
}

void EmbdHost::act(){
    if(act_method == ActMethod::NONE) return;
    Vector2 pos;
    switch(act_method){
        case ActMethod::HUI:
            {
                real_t _t = t * 3;
                real_t r = sin(_t) + sin(_t / 5) + 1.2;
                pos = Vector2(r,0).rotated(_t) + Vector2(5,1.8);
                // logger.println(pos.x,);
            }
            break;
        case ActMethod::GRAB:

        case ActMethod::LISA:
            {
                pos = Vector2(2 * sin(3 * t), 2 * sin(2 * t)) + Vector2(5,2.5);
            }
            break;
        case ActMethod::INTER:
            {
                static constexpr int dur = 1500;
                static constexpr int n = 4;
                uint8_t index = (millis() / dur) % n;
                // pos = Vector2(1.5,0).rotated(index * TAU / n) + Vector2(5,2.5);
                switch(index){
                    case 0:pos = Vector2(0.5,0.5); break;
                    case 1:pos = Vector2(6.5,4.5); break;
                    case 2:pos = Vector2(5.5,0.5); break;
                    case 3:pos = Vector2(0.5,4.5); break;
                }
            }
            break;
        case ActMethod::REP:
            {
                pos = Vector2(2 * sin(3 * t), 2 * sin(4 * t));
            }
        case ActMethod::NONE:
        default:
            break;
    }

    switch(act_method){
        case ActMethod::HUI:
        case ActMethod::GRAB:
        case ActMethod::LISA:
        case ActMethod::REP:
            stepper_x.setTargetPosition(pos.x);
            stepper_y.setTargetPosition(pos.y);
            break;
        case ActMethod::INTER:
            stepper_x.setTargetTrapezoid(pos.x);
            stepper_y.setTargetTrapezoid(pos.y);
        case ActMethod::NONE:
        default:
            break;
    }

}

#endif
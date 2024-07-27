#include "host.hpp"
#include "imgtrans/img_trans.hpp"
#include "match/match.hpp"
#include "interpolation/interpolation.hpp"

#include <algorithm>
#include "match/apriltag.hpp"

using namespace Interpolation;
using namespace NVCV2;

#ifdef CH32V30X

static void fast_diff_opera(Image<Grayscale> & dst, const Image<Grayscale> & src) {
    if((void *)&dst == (void *)&src){
        auto temp = dst.clone();
        fast_diff_opera(temp, src);
        dst = std::move(temp);
        return;
    }

    auto window = dst.get_window().intersection(src.get_window());
    for (auto y = window.y; y < window.y + window.h-1; y++) {
        for (auto x = window.x; x < window.x + window.w-1; x++) {
            const int a = src(Vector2i{x,y});
            const int b = src(Vector2i{x+1,y});
            const int c = src(Vector2i{x,y+1});
            dst[{x,y}] = uint8_t(CLAMP(std::max(
                (ABS(a - c)) * 255 / (a + c),
                (ABS(a - b) * 255 / (a + b))
            ), 0, 255));
        }
    }
}

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
    spi.init(144000000, CommMethod::Blocking, CommMethod::None);
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


    [[maybe_unused]] auto plot_gray = [&](const Image<Grayscale> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.get_size());
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](const Image<Binary> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.get_size());
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_rgb = [&](const Image<RGB565> & src, const Vector2i & pos){
        auto area = Rect2i(pos, src.get_size());
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };



    camera.init();
    camera.setExposureValue(600);


    vl.init();
    vl.enableContMode();
    vl.startConv();


    tcs.init();
    tcs.startConv();
    tcs.setIntegration(48);
    tcs.setGain(TCS34725::Gain::X60);
    ch9141.init();

    stepper_x.setPositionClamp({0.2, 7.8});
    stepper_y.setPositionClamp({0.2, 4.8});
    stepper_z.setPositionClamp({0.2, 26.2});

    auto parseAscii = [&](InputStream & is){
        static String temp;
        while(is.available()){
            auto chr = is.read();
            if(chr == 0) continue;
            temp += chr;
            if(chr == '\n'){
                temp.alphanum();
                parseLine(temp);
                temp = "";
            }
        }
    };

    uart7.bindRxPostCb([&](){parseAscii(uart7);});
    DEBUGGER.bindRxPostCb([&](){parseAscii(DEBUGGER);});

    Matcher matcher;
    auto sketch = make_image<RGB565>(camera.size/2);

    [[maybe_unused]] auto plot_april = [&](const Rect2i & rect, const int index, const int dir){
        painter.bindImage(sketch);
        painter.setColor(RGB565::FUCHSIA);
        painter.drawRoi(rect);
        painter.setColor(RGB565::RED);
        painter.drawString(rect.position + Vector2i{4,4}, toString(index));
        // painter.drawString(rect.position + Vector2i{4, 12}, toString(dir));

        painter.setColor(RGB565::BLUE);
        static constexpr std::array<Vector2i, 4> vecs = {
            Vector2i(12,0),
            Vector2i(0,12),
            Vector2i(-12,0),
            Vector2i(0,-12)
        };

        painter.drawFilledCircle(rect.get_center() + vecs[dir], 3);
        painter.bindImage(tftDisplayer);
    };

    [[maybe_unused]] auto plot_number = [&](const Rect2i & rect, const int index){
        painter.bindImage(sketch);
        painter.setColor(RGB565::GREEN);
        painter.drawRoi(rect);
        painter.setColor(RGB565::YELLOW);
        painter.drawString(rect.position + Vector2i{4,4}, toString(index));
        painter.bindImage(tftDisplayer);
    };

    while(true){
        led = !led;
        sketch.fill(RGB565::BLACK);
        // painter.drawFilledRect(Rect2i{Vector2i{0,0}, img.size}, RGB565::BLACK);


        Image<Grayscale> img = Shape::x2(camera);
        plot_gray(img, {0, img.size.y * 1});
        trans.transmit(img, 0);
    
        auto img_ada = img.space();
        Shape::adaptive_threshold(img_ada, img);
        plot_gray(img_ada, {0, img.size.y * 2});
        // trans.transmit(img_ada, 1);

        auto img_bina = img.space<Binary>();
        Pixels::binarization(img_bina, img_ada, 220);
        Pixels::inverse(img_bina);
        // Shape::dilate(img_bina);
        // plot_bina(img_bina, {0, img.size.y * 2});

        using Shape::FloodFill;
        using Shape::BlobFilter;

        FloodFill ff;
        auto map = ff.run(img_bina, BlobFilter::clamp_area(100, 600));
        Pixels::dyeing(map, map);
        plot_gray(map, Vector2i{0, map.get_size().y * 3});



        for(const auto & blob :ff.blobs()){
            bool has_tag = false;
            bool has_digit = false;
            if(20 < blob.rect.w) has_tag = true;
            else has_digit = true;

            if(has_tag){
                const auto & rect = blob.rect;

                // DEBUG_PRINTLN(rect);


                using Vertexs = std::array<Vector2, 4>;
                Vertexs vertexs;
                {
                    // vertexs[0] = Shape::find_most(map, Pixels::dyeing((Grayscale)blob.index), rect.get_center(), {-1,-1});
                    // vertexs[1] = Shape::find_most(map, Pixels::dyeing((Grayscale)blob.index), rect.get_center(), {1,-1});
                    // vertexs[2] = Shape::find_most(map, Pixels::dyeing((Grayscale)blob.index), rect.get_center(), {-1,1});
                    // vertexs[3] = Shape::find_most(map, Pixels::dyeing((Grayscale)blob.index), rect.get_center(), {1,1});
        
                    vertexs[0] = rect.position;
                    vertexs[1] = rect.position + Vector2i(rect.w, 0);
                    vertexs[2] = rect.position + Vector2i(0, rect.h);
                    vertexs[3] = rect.get_end();
                }

                painter.setColor(RGB565::YELLOW);
                for(const auto & item : vertexs){
                    painter.drawPixel(item);
                }

                static constexpr uint apriltag_s = 4;

                auto get_vertex_val = [&](const Vertexs & _vertexs, const Vector2 & _grid_pos, const Image<Grayscale> & gs) -> Grayscale{

                    auto get_vertex = [&](const Vertexs & __vertexs, const Vector2 & __grid_pos) -> Vector2 {
                        Vector2 grid_scale = (__grid_pos + Vector2{1,1}) / (apriltag_s + 2);

                        Vector2 upper_x = __vertexs[0].lerp(__vertexs[1], grid_scale.x);
                        Vector2 lower_x = __vertexs[2].lerp(__vertexs[3], grid_scale.x);

                        return upper_x.lerp(lower_x, grid_scale.y);
                    };


                    auto get_vertex_grid = [&](const Vertexs & __vertexs, const Vector2 & __grid_pos) -> Vector2{
                        return get_vertex(__vertexs, __grid_pos + Vector2{0.5, 0.5});
                    };

                    return gs[get_vertex_grid(vertexs, _grid_pos)];
                };

                uint16_t code = 0;
                for(uint j = 0; j < apriltag_s; j++){
                    for(uint i = 0; i < apriltag_s; i++){
                        uint16_t mask = (0x8000) >> (j * 4 + i);
                        Grayscale val = get_vertex_val(vertexs, {i,j}, img);
                        if((uint8_t)val > 173) code |= mask;
                    }
                }

                static Apriltag16H5Decoder decoder;
                decoder.update(code);

                plot_april(rect, decoder.index(), decoder.angle());

                Painter<Grayscale> pt;
                auto clipped = img.clone(rect);
                pt.bindImage(clipped);
                pt.drawString({0,0}, toString(decoder.index()));
                pt.drawString({0,8}, toString(decoder.angle()));
                trans.transmit(clipped,1);
            }


            if(has_digit){
                const auto & rect = blob.rect;

                auto char_pos = rect.get_center();
                const Vector2i tmp_size = {8, 12};
                const Rect2i clip_window = Rect2i::from_center(char_pos, tmp_size);
                auto clipped = img.clone(clip_window);


                auto tmp = Shape::x2(clipped);

                painter.setColor(RGB565::BLUE);
                painter.drawRoi(clip_window);

                auto result = matcher.number(tmp, Rect2i(Vector2i(0,0), tmp_size));

                plot_number(clip_window, result);

                Painter<Grayscale> pt;
                pt.bindImage(clipped);
                pt.drawString({0,0}, toString(result));

                trans.transmit(clipped,2);

            }

            if(false){


                // painter.setColor(RGB565::GREEN);
                // // const auto & blobs = ff.blobs();
                // // const auto & blob = blobs[0];
                // painter.setColor(RGB565::GREEN);
                // Rect2i view = Rect2i::from_center(char_pos, Vector2i(14,14));
                // painter.drawRoi(view);
                // // auto piece = img.clone(view);
                // // auto mask = img_bina.clone(view);
                // // Pixels::mask_with(piece, mask);
                // // Shape::gauss(piece);
                // // Pixels::inverse(piece);
                // Mnist mnist;
                // mnist.update(img, char_pos);
                // // logger.println(mnist.outputs);
                // const auto & outputs = mnist.outputs;
                // for(size_t i = 0; i < outputs.size(); i++){
                //     DEBUGGER << outputs[i];
                //     if(i != outputs.size() - 1) DEBUGGER << ',';
                // };
                // DEBUGGER.println();
                // // trans.transmit(piece, 1);
                // // painter.drawString(blob.rect.position, "2");
                // // logger.println(blob.rect, int(blob.rect), blob.index, blob.area);
                // painter.drawString({0,0}, String(mnist.output.token));
            }


            // {
            //     painter.setColor(RGB565::YELLOW);
            //     painter.drawRoi({0,0,28,28});
            // }

        }

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
        plot_rgb(sketch, {0,0});
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
    
        case "dz"_ha:
            if(args.size() == 1){
                stepper_z.setTargetSpeed(real_t(args[0]));
            }

        case "dxy"_ha:
            if(args.size() == 2){
                steppers.x.setTargetSpeed(real_t(args[0]));
                steppers.y.setTargetSpeed(real_t(args[1]));
                DEBUG_PRINTLN(args[0], args[1]);
            }
            break;

        case "nz"_ha:
            if(args.size() == 1){
                steppers.y.set_nozzle(int(args[0]));
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
        case "frz"_ha:
            trigger_method(stepper_w.freeze);
        default:
            CliAP::parseTokens(_command, args);
            break;
    }
}

void EmbdHost::reset(){
    steppers.x.reset();
    steppers.y.reset();
    steppers.z.reset();
    delay(10);
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
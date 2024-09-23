#include "host.hpp"
#include "imgtrans/img_trans.hpp"
#include "match/match.hpp"
#include "algo/interpolation/interpolation.hpp"

#include "sys/math/float/bf16.hpp"

#include <algorithm>
#include "match/apriltag/dec16h5.hpp"

#include "motions/move.hpp"
#include "motions/pickdrop.hpp"

#include "hal/bus/spi/spihw.hpp"

using namespace NVCV2;

#ifdef CH32V30X



void EmbdHost::main(){

    auto & lcd_blk = portC[7];
    auto & spi = spi2;
    
    run_led.outpp();
    empty_led.outpp();
    busy_led.outpp();
    lcd_blk.outpp(1);

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
    tftDisplayer.fill(RGB565::BLACK);
    i2c.init(400000);

    camera.init();
    camera.setExposureValue(1200);

    toggle_key.init();

    vl.init();
    vl.enableContMode();
    vl.startConv();


    tcs.init();
    tcs.startConv();
    tcs.setIntegration(48);
    tcs.setGain(TCS34725::Gain::X60);
    ch9141.init();

    stepper_x.setPositionLimit({0.2, 7.8});
    stepper_y.setPositionLimit({0.2, 4.8});
    stepper_z.setPositionLimit({0.2, 24.2});


    Matcher matcher;
    auto sketch = make_image<RGB565>(camera.get_size()/2);

    using Vertexs = std::array<Vector2, 4>;

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

    [[maybe_unused]] auto plot_roi = [&](const Rect2i & rect){
        painter.bindImage(sketch);
        painter.setColor(RGB565::CORAL);
        painter.drawRoi(rect);
    };

    [[maybe_unused]] auto plot_april = [&](const Vertexs & vertex, const int index, const real_t dir){
        painter.bindImage(sketch);
        painter.setColor(RGB565::FUCHSIA);

        painter.drawPolygon(vertex.begin(), vertex.size());
        auto rect = Rect2i(vertex.begin(), vertex.size());
        painter.setColor(RGB565::RED);
        painter.drawString(rect.position + Vector2i{4,4}, toString(index));

        painter.setColor(RGB565::BLUE);
        painter.drawFilledCircle(rect.get_center() + Vector2(12, 0).rotated(dir), 3);
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


    // resetSlave();
    // delay(900);
    bindSystickCb([&](){this->tick();});
    // steppers.do_home();

    painter.setChFont(font7x7);
    while(true){
        // painter.setColor(RGB565::WHITE);
        // painter.drawString({0, 0}, "进入 设置 启动");
        // painter.drawString({0, 8}, "开始 时间 设定 确认");
        // painter.drawString({0, 16}, "选中 缩放 打开 关闭");
        sketch.fill(RGB565::BLACK);

        Image<Grayscale> img = Shape::x2(camera);
        // auto img = raw_img.space();
        // Geometry::perspective(img, raw_img);
        plot_gray(img, {0, img.get_size().y * 1});
        trans.transmit(img, 0);
    
        auto img_ada = img.space();
        // Shape::adaptive_threshold(img_ada, img);
        // plot_gray(img_ada, {0, img.get_size().y * 2});

        auto img_processed = img.space<Grayscale>();
        // Shape::canny(img_bina, img, {60, 120});
        Shape::eye(img_processed, img);
        auto img_processed2 = img.space<Grayscale>();
        Shape::eye(img_processed2, img_processed);
        // Pixels::binarization(img_bina, img_ada, 220);
        // Pixels::inverse(img_bina);
        plot_gray(img_processed, {0, img.get_size().y * 2});
        plot_gray(img_processed2, {0, img.get_size().y * 3});
    }
}

void EmbdHost::parseCommand(const NodeId id, const Command cmd, const CanMsg &msg){
    // steppers.x.parseCommand(id, cmd, msg);
    // steppers.y.parseCommand(id, cmd, msg);
    // steppers.z.parseCommand(id, cmd, msg);
}


void EmbdHost::resetSlave(){
    // steppers.x.reset();
    // steppers.y.reset();
    // steppers.z.reset();
}
void EmbdHost::resetAll(){
    resetSlave();
    delay(10);
    Sys::Misc::reset();
}

void EmbdHost::cali(){
    // steppers.w.triggerCali();
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


void EmbdHost::tick(){

    // steppers.tick();

    // toggle_key.update();
    // if(toggle_key.pressed()){
    //     steppers.toggle_nz();

    // }

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

    parseAscii(uart7);
    parseAscii(uart2);

    // auto index = (millis() % 3);
    // steppers[index].updateAll();
    // steppers.z.updateAll();
    readCan();
    act();
}

#endif
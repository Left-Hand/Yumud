#include "host.hpp"
#include "imgtrans/img_trans.hpp"
#include "match/match.hpp"
#include "algo/interpolation/interpolation.hpp"
#include "robots/foc/stepper/protocol/ascii_protocol.hpp"

#include "types/float/bf16.hpp"

#include <algorithm>
#include "match/apriltag/dec16h5.hpp"

#include "motions/move.hpp"
#include "motions/pickdrop.hpp"

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

    camera.init();
    camera.setExposureValue(1000);

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


    resetSlave();
    delay(900);
    bindSystickCb([&](){this->tick();});
    steppers.do_home();


    while(true){

        sketch.fill(RGB565::BLACK);

        Image<Grayscale> img = Shape::x2(camera);
        plot_gray(img, {0, img.get_size().y * 1});
        trans.transmit(img, 0);
    
        auto img_ada = img.space();
        Shape::adaptive_threshold(img_ada, img);
        plot_gray(img_ada, {0, img.get_size().y * 2});

        auto img_bina = img.space<Binary>();
        Pixels::binarization(img_bina, img_ada, 220);
        Shape::canny(img_bina, img, {40,100});
        Pixels::inverse(img_bina);

        using Shape::FloodFill;
        using Shape::BlobFilter;

        FloodFill ff;
        auto map = ff.run(img_bina, BlobFilter::clamp_area(400, 1600));
        Pixels::dyeing(map, map);
        plot_gray(map, Vector2i{0, map.get_size().y * 3});


        for(const auto & blob :ff.blobs()){
            if(true){
        
                bool is_tag = false;
                bool is_digit = false;
                if(2000 < blob.rect.w) is_tag = true;//make it impossible
                if(2000 < blob.rect.w) is_digit = true;//make it impossible

                const auto & rect = blob.rect;

                if(is_tag){

                    static constexpr uint apriltag_s = 4;

                    auto get_vertex_val = [&](const Vertexs & _vertexs, const Vector2 & _grid_pos, const Image<Grayscale> & gs) -> Grayscale{

                        auto get_vertex = [&](const Vertexs & __vertexs, const Vector2 & __grid_pos) -> Vector2 {
                            Vector2 grid_scale = (__grid_pos + Vector2{1,1}) / (apriltag_s + 2);

                            Vector2 upper_x = __vertexs[0].lerp(__vertexs[1], grid_scale.x);
                            Vector2 lower_x = __vertexs[3].lerp(__vertexs[2], grid_scale.x);

                            return upper_x.lerp(lower_x, grid_scale.y);
                        };


                        auto get_vertex_grid = [&](const Vertexs & __vertexs, const Vector2 & __grid_pos) -> Vector2{
                            return get_vertex(__vertexs, __grid_pos + Vector2{0.5, 0.5});
                        };

                        return gs.bilinear_interpol(get_vertex_grid(_vertexs, _grid_pos));
                    };

                    auto find_vertex = [](const Image<Grayscale> & __map, const Grayscale & match, const Rect2i & roi) -> Vertexs{
                        auto x_range = roi.get_x_range();
                        auto y_range = roi.get_y_range();

                        Vertexs ret;
                        auto center = roi.get_center();
                
                        for(auto & item : ret){
                            item = center;
                        }
        
                        #define COMP(s1, s2, i)\
                        if((0 s1*x) + (0 s2*y) < (0 s1*ret[i].x) + (0 s2*ret[i].y))\
                        ret[i] = Vector2i(x,y);

                        for(auto y = y_range.from; y < y_range.to; ++y){
                            for(auto x = x_range.from; x < x_range.to; ++x){
                                auto color = __map[{x,y}];
                                if(color != match) continue;

                                COMP(-1, -1, 0)
                                COMP(+1, -1, 1)
                                COMP(+1, +1, 2)
                                COMP(-1, +1, 3)
                            }
                        }

                        return ret;
                    };

                    auto vertexs = find_vertex(map, Pixels::dyeing((Grayscale)blob.index), rect);

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

                    plot_april(vertexs, decoder.index(), decoder.direction() * PI / 2 + (vertexs[1] - vertexs[0]).angle());

                    Painter<Grayscale> pt;
                    auto clipped = img.clone(rect);
                    pt.bindImage(clipped);
                    pt.drawString({0,0}, toString(decoder.index()));
                    pt.drawString({0,8}, toString(decoder.direction()));
                    trans.transmit(clipped,1);
                }


                if(is_digit){


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

            }
        }

        plot_rgb(sketch, {0,0});

        // DEBUG_PRINTLN(steppers.x_axis.readMM(), steppers.y_axis.readMM(), steppers.z_axis.readMM());
        static uint last_turn = 0;
        uint this_turn = millis() / 100;
        if(last_turn != this_turn){
            // DEBUG_PRINTS("busy ", actions.pending());
            // ch9141.prints("busy ", actions.pending());
            DEBUG_PRINTLN(bool(toggle_key));
            last_turn = this_turn;
            run_led = !run_led;
        }
        busy_led = actions.pending();

    }
}

void EmbdHost::parseCommand(const NodeId id, const Command cmd, const CanMsg &msg){
    steppers.x.parseCommand(id, cmd, msg);
    steppers.y.parseCommand(id, cmd, msg);
    steppers.z.parseCommand(id, cmd, msg);
}


void EmbdHost::resetSlave(){
    steppers.x.reset();
    steppers.y.reset();
    steppers.z.reset();
}
void EmbdHost::resetAll(){
    resetSlave();
    delay(10);
    Sys::Misc::reset();
}

void EmbdHost::cali(){
    steppers.w.triggerCali();
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
    actions.update();

    toggle_key.update();
    if(toggle_key.pressed()){
        steppers.toggle_nz();
    }

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

    auto index = (millis() % 3);
    steppers[index].updateAll();
    // steppers.z.updateAll();
    readCan();
}

#endif
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

using namespace yumud::nvcv2;

using Vector2i = Vector2_t<int>;

#define WHARE_OK while(true){DEBUGGER.println(millis());};


using Sys::t;

using namespace yumud;
using namespace yumud::drivers;


class Node{
protected:
    using Nodes = std::vector<std::reference_wrapper<Node>>;
    Nodes children_;

public:

    void addChild(Node & node){
        children_.push_back(node);
    }

    auto getChildren(){
        return children_;
    }
};


struct Theme{
    RGB888 stoke_color;
    RGB888 bg_color;
    RGB888 text_color;
};

// scexpr auto a = sizeof(Theme);

class CanvasItem{
public:
    // struct Config{
    Rect2i rect = Rect2i();
    // };  
    // auto config() const{return config_;}
protected:
    // Config & config_;
    const Theme & theme_;
public:
    CanvasItem(const Theme & _theme): theme_(_theme){;}
    virtual void render(PainterConcept & painter) = 0;
};

#define PASS_THEME(derived, base)\
derived(const Theme & _theme): base(_theme){;}

class Control:public CanvasItem{
public:
    // CanvasItem(const Theme & _theme): CanvasItem(_theme){;}
    PASS_THEME(Control, CanvasItem)
};

class Label:public Control{
public:
    String text;
    PASS_THEME(Label, Control)

    void render(PainterConcept & painter) override{
        painter.setColor(theme_.bg_color);
        painter.drawFilledRect(rect);

        painter.setColor(theme_.stoke_color);
        painter.drawHollowRect(rect);

        painter.setColor(theme_.text_color);
        painter.drawString(rect.position + Vector2i(10,7), text);
    }
};

// class ButtonBase:public Control{
    
// };


class Slider:public Control{
public:
    PASS_THEME(Slider, Control)

    Range range;
    
    void render(PainterConcept & painter) override{
        painter.setColor(theme_.bg_color);
        painter.drawFilledRect(rect);

        painter.setColor(theme_.stoke_color);
        painter.drawHollowRect(rect);
        
        scexpr auto sp = 3;
        auto sb = rect.position + Vector2i{sp, rect.size.y/2};
        auto sw = rect.size.x - 2 * sp;

        painter.setColor(theme_.stoke_color);
        painter.drawFilledRect(Rect2i{sb, Vector2i{sw, 2}});

        scexpr auto h = 6;
        scexpr auto w = 6;
        painter.setColor(theme_.text_color);
        painter.drawFilledRect(Rect2i{sb + Vector2i{5, - h / 2}, Vector2i{w, h}});
    }
};


class OptionButton:public Control{
public:
    PASS_THEME(OptionButton, Control)
    
    void render(PainterConcept & painter) override{
        painter.setColor(theme_.bg_color);
        painter.drawFilledRect(rect);

        painter.setColor(theme_.stoke_color);
        painter.drawHollowRect(rect);

        painter.setColor(theme_.stoke_color);
        painter.drawFilledRect(Rect2i{rect.position + Vector2i(3,6), Vector2i(22,10)});

        painter.setColor(theme_.text_color);
        painter.drawFilledCircle(rect.position + Vector2i(10,10), 5);

        painter.setColor(theme_.text_color);
        painter.drawString(rect.position + Vector2i(30,7), "选择");

        
    }
};

class Eye:public CanvasItem{
public:
    struct Config{
        Vector2i l_center;
        Vector2i r_center;

        size_t eye_radius;
        size_t iris_radius;
        size_t pupil_radius;
    };

protected:
    const Config & config_;

    std::array<Vector2, 2> eye_pos;
public:
    Eye(const Theme & theme, const Config & config):CanvasItem(theme), config_(config){}

    void setEye(const Vector2 & l_pos, const Vector2 & r_pos){
        eye_pos[0] = l_pos;
        eye_pos[1] = r_pos;
    }

    void render(PainterConcept & painter) override{
        auto render_eye = [&](const LR side){

            auto center = (side == LR::LEFT) ? config_.l_center : config_.r_center;
            auto center_p = center + ((side == LR::LEFT) ? eye_pos[0] : eye_pos[1]) * config_.eye_radius * real_t(0.5);

            painter.setColor(ColorEnum::WHITE);
            painter.drawFilledCircle(center, config_.eye_radius);

            painter.setColor(ColorEnum::BROWN);
            painter.drawFilledCircle(center_p, config_.iris_radius);

            painter.setColor(ColorEnum::BLACK);
            painter.drawFilledCircle(center_p, config_.pupil_radius);
        };

        render_eye(LR::LEFT);
        render_eye(LR::RIGHT);
    }
};

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

    // auto a = i8{8};
    // auto b = a + i8{8};
 
    Slider slider{theme};

    OptionButton opt{theme};
    
    I2cSw       i2c{portD[2], portC[12]};
    i2c.init(1000000);
    
    MT9V034 camera{i2c};
    camera.init();
    camera.setExposureValue(1200);
    // label.rect = Rect2i{20 + 10,20,100,20};
    // label2.rect = Rect2i{20,60 + 20,100,20};

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
        // painter.bindImage(sketch);
        // painter.setColor(ColorEnum::RED);
        // painter.drawRoi(rect);
    };

    // [[maybe_unused]] auto plot_april = [&](const Vertexs & vertex, const int index, const real_t dir){
    //     painter.bindImage(sketch);
    //     painter.setColor(RGB565::FUCHSIA);

    //     painter.drawPolygon(vertex.begin(), vertex.size());
    //     auto rect = Rect2i(vertex.begin(), vertex.size());
    //     painter.setColor(RGB565::RED);
    //     painter.drawString(rect.position + Vector2i{4,4}, toString(index));

    //     painter.setColor(RGB565::BLUE);
    //     painter.drawFilledCircle(rect.get_center() + Vector2(12, 0).rotated(dir), 3);
    //     painter.bindImage(tftDisplayer);
    // };

    [[maybe_unused]] auto plot_number = [&](const Rect2i & rect, const int index){
        // painter.bindImage(sketch);
        painter.setColor(ColorEnum::GREEN);
        painter.drawRoi(rect);
        painter.setColor(ColorEnum::YELLOW);
        painter.drawString(rect.position + Vector2i{4,4}, toString(index));
        painter.bindImage(tftDisplayer);
    };

    auto eye_conf =         Eye::Config{
            .l_center = Vector2i{62, 60},
            .r_center = Vector2i{62, 180},

            .eye_radius = 40,
            .iris_radius = 13,
            .pupil_radius = 7
    };

    Eye eye{
        theme,
        eye_conf
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

        eye.setEye(Vector2::RIGHT.rotated(t * tau), Vector2::RIGHT.rotated(t * pi));;

        delay(20);
        painter.fill(ColorEnum::BLACK);

        // DEBUG_PRINTLN(millis());
        // PANIC("why");
        #endif
    
    }
}
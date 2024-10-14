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

    painter.setChFont(font7x7);
    painter.setEnFont(font8x5);

    Theme theme{
        .stoke_color =  {70,70,70},
        .bg_color =     {10,10,10},
        .text_color =   Colors::PINK
    };
    
    Label label{theme};
    label.text = "hello world";


    Label label2{theme};
    label2.text = String("你好世界");
 
    Slider slider{theme};

    OptionButton opt{theme};
    // label.rect = Rect2i{20 + 10,20,100,20};
    // label2.rect = Rect2i{20,60 + 20,100,20};
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
        painter.setColor(Colors::BLUE);
        // painter.drawHollowRect(rect);
        // painter.drawFilledRect(rect);
        painter.drawFilledCircle(rect.position, 5);

        // painter.drawString({0,0}, "hello");
        // painter.drawFilledRect(rect);
        logger.println(rect, tftDisplayer.get_view().intersection(rect));

        // logger.println(millis());
        #endif

        label.rect = Rect2i{15 + 10 * sin(t),20,100,20};
        label2.rect = Rect2i{15,80 + 20 * sin(t),100,20};
        slider.rect = Rect2i{15,120,100,20};
        opt.rect = Rect2i{15,160,100,20};

        label.render(painter);
        label2.render(painter);
        slider.render(painter);
        opt.render(painter);

        delay(10);
        tftDisplayer.fill(RGB565::BLACK);


        logger.println(label.rect, label2.rect);
    }
}
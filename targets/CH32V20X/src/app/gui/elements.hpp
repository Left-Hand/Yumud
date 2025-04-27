#pragma once

#include "types/rgb.h"
#include "core/string/string.hpp"


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

class Signal {
private:
    using Callback = std::function<void()>;
    using Callbacks = std::vector<Callback>; 

    Callbacks callbacks_;

public:
    void connect(const Callback & callback) {
        callbacks_.push_back(callback);
    }

    void emit() {
        for (auto& callback : callbacks_) {
            callback();
        }
    }
};

struct Theme{
    RGB888 stroke_color;
    RGB888 bg_color;
    RGB888 text_color;

    static Theme defaultTheme() {
        return Theme{
            .stroke_color = RGB888(0, 0, 0),
            .bg_color = RGB888(255, 255, 255),
            .text_color = RGB888(0, 0, 0)
        };
    }
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

        painter.setColor(theme_.stroke_color);
        painter.drawHollowRect(rect).unwrap();

        painter.setColor(theme_.text_color);
        painter.drawString(rect.position + Vector2i(10,7), text).unwrap();
    }
};

// class ButtonBase:public Control{
    
// };


class Slider:public Control{
public:
    PASS_THEME(Slider, Control)

    Range_t<real_t> range;
    
    void render(PainterConcept & painter) override{
        painter.setColor(theme_.bg_color);
        painter.drawFilledRect(rect);

        painter.setColor(theme_.stroke_color);
        painter.drawHollowRect(rect).unwrap();
        
        scexpr auto sp = 3;
        auto sb = rect.position + Vector2i{sp, rect.size.y/2};
        auto sw = rect.size.x - 2 * sp;

        painter.setColor(theme_.stroke_color);
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

        painter.setColor(theme_.stroke_color);
        painter.drawHollowRect(rect).unwrap();

        painter.setColor(theme_.stroke_color);
        painter.drawFilledRect(Rect2i{rect.position + Vector2i(3,6), Vector2i(22,10)});

        painter.setColor(theme_.text_color);
        painter.drawFilledCircle(rect.position + Vector2i(10,10), 5).unwrap();

        painter.setColor(theme_.text_color);
        painter.drawString(rect.position + Vector2i(30,7), "选择").unwrap();

        
    }
};
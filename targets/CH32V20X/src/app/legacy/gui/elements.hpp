#pragma once

#include "primitive/colors/rgb/rgb.hpp"
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

// static constexpr auto a = sizeof(Theme);

class CanvasItem{
public:
    // struct Config{
    math::Rect2u rect = math::Rect2u();
    // };  
    // auto config() const{return config_;}
protected:
    // Config & config_;
    const Theme & theme_;
public:
    CanvasItem(const Theme & _theme): theme_(_theme){;}
    virtual void render(PainterBase & painter) = 0;
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

    void render(PainterBase & painter) override{
        painter.set_color(theme_.bg_color);
        painter.draw_filled_rect(rect).examine();

        painter.set_color(theme_.stroke_color);
        painter.draw_hollow_rect(rect).examine();

        painter.set_color(theme_.text_color);
        painter.draw_string(rect.position + Vec2u(10,7), StringView(text)).examine();
    }
};

// class ButtonBase:public Control{
    
// };


class Slider:public Control{
public:
    PASS_THEME(Slider, Control)

    Range2<iq16> range;
    
    void render(PainterBase & painter) override{
        painter.set_color(theme_.bg_color);
        painter.draw_filled_rect(rect).examine();

        painter.set_color(theme_.stroke_color);
        painter.draw_hollow_rect(rect).examine();
        
        static constexpr auto sp = 3u;
        auto sb = rect.position + Vec2u{sp, rect.size.y/2};
        auto sw = rect.size.x - 2 * sp;

        painter.set_color(theme_.stroke_color);
        painter.draw_filled_rect(math::Rect2u{sb, Vec2u{sw, 2}}).examine();

        static constexpr auto h = 6u;
        static constexpr auto w = 6u;
        painter.set_color(theme_.text_color);
        painter.draw_filled_rect(math::Rect2u{sb + Vec2i{5, - h / 2}, Vec2u{w, h}}).examine();
    }
};


class OptionButton:public Control{
public:
    PASS_THEME(OptionButton, Control)
    
    void render(PainterBase & painter) override{
        painter.set_color(theme_.bg_color);
        painter.draw_filled_rect(rect).examine();

        painter.set_color(theme_.stroke_color);
        painter.draw_hollow_rect(rect).examine();

        painter.set_color(theme_.stroke_color);
        painter.draw_filled_rect(math::Rect2u{rect.position + Vec2u(3,6), Vec2u(22,10)}).examine();

        painter.set_color(theme_.text_color);
        painter.draw_filled_circle(rect.position + Vec2u(10,10), 5).examine();

        painter.set_color(theme_.text_color);
        painter.draw_string(rect.position + Vec2u(30,7), "选择").examine();

        
    }
};
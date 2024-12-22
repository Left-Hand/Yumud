#include "tb.h"
#include "sys/debug/debug_inc.h"
#include "sys/stream/StringStream.hpp"

#include "sys/polymorphism/proxy.hpp"

using namespace ymd;

PRO_DEF_MEM_DISPATCH(MemDraw, Draw);
PRO_DEF_MEM_DISPATCH(MemArea, Area);

struct Drawable : pro::facade_builder
    ::add_convention<MemDraw, void(OutputStream& output)>
    ::add_convention<MemArea, real_t()>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

class Rectangle {
public:
    Rectangle(real_t width, real_t height) : width_(width), height_(height) {}
    Rectangle(const Rectangle&) = default;

    void Draw(OutputStream& out) const {
        out << "{Rectangle: width = " << width_ << ", height = " << height_ << "}";
    }
    real_t Area() const { return width_ * height_; }

private:
    real_t width_;
    real_t height_;
};

class Circle {
public:
    Circle(real_t radius) : radius_(radius){}
    Circle(const Circle&) = default;

    void Draw(OutputStream& out) const {
        out << "{Circle: radius = " << radius_ << "}";
    }
    real_t Area() const { return radius_ * radius_ * real_t(PI);}

private:
    real_t radius_;
};

String PrintDrawableToString(pro::proxy<Drawable> p) {
    StringStream result;
    result << "entity = ";
    p->Draw(result);
    result << ", area = " << p->Area();
    return std::move(result).move_str();
    // return result.move_str();
}

void proxy_tb(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD);

    pro::proxy<Drawable> p = pro::make_proxy<Drawable, Rectangle>(3, 5);
    pro::proxy<Drawable> p2 = pro::make_proxy<Drawable, Circle>(3);
    DEBUG_PRINTLN(PrintDrawableToString(p));  // Prints: "entity = {Rectangle: width = 3.00, height = 5.00}, area = 15"
    DEBUG_PRINTLN(PrintDrawableToString(p2));  // Prints: "entity = {Circle: radius = 3.00}, area = 28.27"

    while(true);
}

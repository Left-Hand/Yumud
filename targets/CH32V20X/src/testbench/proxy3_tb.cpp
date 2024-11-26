#include "tb.h"
#include "sys/debug/debug_inc.h"
#include "sys/stream/StringStream.hpp"

#include "sys/polymorphism/proxy.hpp"

using namespace ymd;

PRO_DEF_MEM_DISPATCH(MemDraw, Draw);
PRO_DEF_MEM_DISPATCH(MemArea, Area);

struct Drawable : pro::facade_builder
    ::add_convention<MemDraw, void(ymd::OutputStream& output)>
    ::add_convention<MemArea, real_t()>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

class Rectangle {
public:
    Rectangle(real_t width, real_t height) : width_(width), height_(height) {}
    Rectangle(const Rectangle&) = default;

    void Draw(ymd::OutputStream& out) const {
        out << "{Rectangle: width = " << width_ << ", height = " << height_ << "}";
    }
    real_t Area() const { return width_ * height_; }

private:
    real_t width_;
    real_t height_;
};

String PrintDrawableToString(pro::proxy<Drawable> p) {
    ymd::StringStream result;
    result << "entity = ";
    p->Draw(result);
    result << ", area = " << p->Area();
    return std::move(result).str();
}

void proxy_tb(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD);

    pro::proxy<Drawable> p = pro::make_proxy<Drawable, Rectangle>(3, 5);
    String str = PrintDrawableToString(p);
    DEBUGGER << str << "\n";  // Prints: "entity = {Rectangle: width = 3, height = 5}, area = 15"
}

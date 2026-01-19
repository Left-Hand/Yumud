#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/stream/BufStream.hpp"

#include "core/polymorphism/proxy.hpp"
#include "core/math/real.hpp"

#include "core/clock/time.hpp"


using namespace ymd;

PRO_DEF_MEM_DISPATCH(MemDraw, Draw);
PRO_DEF_MEM_DISPATCH(MemArea, Area);

struct Drawable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"<<", true>, OutputStream&(OutputStream&) const>
    // ::add_convention<MemArea, real_t() const>
    ::build {};



class CopyabilityReflector {
 public:
  template <class T>
  constexpr explicit CopyabilityReflector(std::in_place_type_t<T>)
      : copyable_(std::is_copy_constructible_v<T>) {}

  template <class F, class R>
  struct accessor {
    bool IsCopyable() const noexcept {
      const CopyabilityReflector& self = pro::proxy_reflect<R>(pro::access_proxy<F>(*this));
      return self.copyable_;
    }
  };

 private:
  bool copyable_;
};

struct CopyabilityAware : pro::facade_builder
    ::add_direct_reflection<CopyabilityReflector>
    ::build {};

// volatile int cnt = 0;
class Rectangle {
public:
    Rectangle(real_t width, real_t height) : width_(width), height_(height) {
        // DEBUG_PRINTLN("rect created");
        // cnt++;
    }
    Rectangle(const Rectangle&) = delete;
    // Rectangle(const Rectangle&) = default;
    // Rectangle(Rectangle &&) = default;
    Rectangle(Rectangle &&) = delete;

    ~Rectangle(){
        // DEBUG_PRINTLN("rect dropped");
    }
    real_t Area() const { return width_ * height_; }

    // OutputStream & operator <<(OutputStream & os) const{
    //     return os << "{Rectangle: width = " << width_ << ", height = " << height_ << "}";
    // }

    friend OutputStream & operator<<(OutputStream & os, const Rectangle & rect) {
        return os << "{Rectangle: width = " << rect.width_ << ", height = " << rect.height_ << "}";
    }

    auto & width() { return width_; }
private:
    real_t width_;
    real_t height_;

    // std::array<uint32_t, 1000> data;
};

class Circle {
public:
    Circle(real_t radius) : radius_(radius){
        // DEBUG_PRINTLN("circle created");
        // cnt++;
    }
    Circle(const Circle&) = delete;
    Circle(Circle &&) = delete;
    ~Circle(){
        // DEBUG_PRINTLN("circle dropped");
    }

    real_t Area() const { return radius_ * radius_ * real_t(PI);}
    // OutputStream & operator <<(OutputStream & os) const{
    //     return os << "{Circle: radius = " << radius_ << "}";
    // }

    friend OutputStream & operator<<(OutputStream & os, const Circle & circle) {
        return os << "{Circle: radius = " << circle.radius_ << "}";
    }
private:
    real_t radius_;
};

// OutputStream & ymd::operator <<(OutputStream & os) const{
//     return os << "{Circle: radius = " << radius_ << "}";
// }

// OutputStream & ymd::operator <<(OutputStream & os, ){
//     return os << "{Circle: radius = " << radius_ << "}";
// }

struct What{

};

// __fast_inline String PrintDrawableToString(const pro::proxy<Drawable> & p) {
//     StringStream result;
//     result << "entity = ";
//     p->Draw(result);
//     result << ", area = " << p->Area();
//     return std::move(result).move_str();
//     // return result.move_str();
// }

using DrawableProxy = pro::proxy<Drawable>;

// void myswap(DrawableProxy& a, DrawableProxy& b) {
//     auto && temp = std::move(a);  // 或者是 copy，取决于类型的支持
//     a = std::move(b);
//     b = std::move(temp);
// }
// using np = std::nullptr;
void proxy_tb(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD);
    DEBUG_PRINTLN("\r\nstart");
    std::array<char, 1000> buf;
    BufStream ss{std::span(buf)};
    Microseconds dur;
    {

        std::vector<DrawableProxy> shapes;
        shapes.reserve(2000);



        // auto p1 = pro::make_proxy<Drawable, Rectangle>(3, 5);
        // auto p2 = pro::make_proxy<Drawable, Circle>(3);

        auto rect = Rectangle(3,5);
        auto circle = Circle(3);

        for(size_t i = 0; i < 1000; i++){
            // std::swap(p1, p2);
            // swap(p1,p2);
            // pro::make_proxy<Drawable, Rectangle>(3, 5);
            // pro::make_proxy<Drawable, Circle>(3);

            // shapes.emplace_back(pro::make_proxy<Drawable, Rectangle>(3, 5));
            // shapes.emplace_back(pro::make_proxy<Drawable, Circle>(3));

            // shapes.emplace_back(pro::make_proxy<Drawable, Rectangle>(rect));
            // shapes.emplace_back(pro::make_proxy<Drawable, Circle>(circle));

            // shapes.emplace_back(&rect);
            // shapes.emplace_back(&circle);

            shapes.push_back(&rect);
            shapes.push_back(&circle);

            // new Rectangle(3,5);
            // new Circle(3);

            // Rectangle(3,5);
            // Circle(3);
        }


        const auto m = clock::micros();
        
        // char str_buf[16] = {0};
        for(size_t i = 0; i < 100000; i++){
            // ss.println(*shapes[i]);
            // strconv::qtoa(clock::seconds(), str_buf, 4);
            // DEBUG_PRINTLN(*shapes[i]);
            // rect.width() = rect.width() + 1;
            // DEBUG_PRINTLN(rect);
        }
        dur = clock::micros() - m;

        // using type = decltype(nullptr);
        // std::nullptr_t
        // std::nullopt
        // sizeof(DrawableProxy);
        // for(const auto & shape:shapes) {
        // for(const auto & shape:shapes) {
        // }

        // DEBUG_PRINTLN(sizeof(decltype(p)));

        // pro::proxy<CopyabilityAware> p1 = std::make_unique<int>();
        // DEBUGGER << std::boolalpha << p1.IsCopyable() << "\n";  // Prints: "false"

        // pro::proxy<CopyabilityAware> p2 = std::make_shared<int>();
        // DEBUGGER << p2.IsCopyable() << "\n";  // Prints: "true"
    }

    DEBUG_PRINTLN("!!!!");
    DEBUG_PRINTLN(StringView(ss));
    DEBUG_PRINTLN("!!!!");
    clock::delay(100ms);
    DEBUG_PRINTLN("done", dur);

    // while(true){
    //     DEBUG_PRINTLN(13 * sin(13 * t));
    //     clock::delay(1ms);
    // }

    while(true);
}

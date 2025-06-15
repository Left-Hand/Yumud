#include "core/debug/debug.hpp"
#include "core/stream/StringStream.hpp"

#include "core/math/real.hpp"

#include "core/clock/time.hpp"
#include "core/polymorphism/metaclass.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;


Interface(area,
    Fn((), real_t)
    // ,Fn((), void)
); 

Metaclass(Myclass, area);

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
    real_t area() const { return width_ * height_; }
    real_t area(const int i) const { return i * height_; }

    friend OutputStream & operator<<(OutputStream & os, const Rectangle & rect) {
        return os << "{Rectangle: width = " << rect.width_ << ", height = " << rect.height_ << "}";
    }

    auto & width() { return width_; }
private:
    real_t width_;
    real_t height_;
};

class Circle {
public:
    Circle(real_t radius) : radius_(radius){
    }
    Circle(const Circle&) = delete;
    Circle(Circle &&) = delete;
    ~Circle(){
    }
    real_t area(const int i) const { return radius_; }
    real_t area() const { return radius_ * radius_ * real_t(PI);}

    friend OutputStream & operator<<(OutputStream & os, const Circle & circle) {
        return os << "{Circle: radius = " << circle.radius_ << "}";
    }
private:
    real_t radius_;
};

void test(Myclass obj){
    obj.area();
}


void metaclass_tb(){
    hal::uart2.init({576000});
    DEBUGGER.retarget(&hal::uart2);

    DEBUG_PRINTLN("\r\nstart");

    Rectangle rect = {1,1};
    Circle circle = {1};

    test(rect);
    test(circle);


}

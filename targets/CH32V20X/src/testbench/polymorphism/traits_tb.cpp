#include "core/debug/debug.hpp"

#include "core/math/real.hpp"
#include "core/clock/time.hpp"

#include "core/polymorphism/traits.hpp"

#include "hal/bus/uart/hw_singleton.hpp"

using namespace ymd;



struct AreaTrait{
private: 
    template<typename T> 
    struct vtable_t {
        using Self = vtable_t<T>;
        static iq16 area_dispatcher (const void* self ) {
            return (reinterpret_cast<const T *>(self))->area ( ); 
        };

        static constexpr iq16 (* area) (const void* self ) = &Self::area_dispatcher;
    }; 

    struct vtable{
        iq16 (* area) (const void* self );
    }; 

    struct body{
        public:
        inline iq16 area ( ) const{
            return vtable_-> area (self_);
        } 

        body(const void * self, vtable * table):
            self_(self), vtable_(table){}
    private:
        const void * self_;
        vtable * vtable_;
    };

    body body_;

public:
    AreaTrait() = delete;

    body * operator ->(){
        return &body_;
    }
    
    template<typename T>
    AreaTrait(T * obj):
        body_(
            reinterpret_cast<void *>(obj), 
            {[]{static vtable_t<T> impl; return reinterpret_cast<vtable*>(&impl);}()}
        ){}
    
};

class Rectangle {
public:
    Rectangle(iq16 width, iq16 height) : width_(width), height_(height) {
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
    iq16 area() const { return width_ * height_; }

    friend OutputStream & operator<<(OutputStream & os, const Rectangle & rect) {
        return os << "{Rectangle: width = " << rect.width_ << ", height = " << rect.height_ << "}";
    }

    auto & width() { return width_; }
private:
    iq16 width_;
    iq16 height_;
};

class Circle {
public:
    Circle(iq16 radius) : radius_(radius){
    }
    Circle(const Circle&) = delete;
    Circle(Circle &&) = delete;
    ~Circle(){
    }

    iq16 area() const { return radius_ * radius_ * iq16(M_PI);}

    friend OutputStream & operator<<(OutputStream & os, const Circle & circle) {
        return os << "{Circle: radius = " << circle.radius_ << "}";
    }
private:
    iq16 radius_;
};


void area(AreaTrait obj){
    DEBUG_PRINTLN(obj->area());

    // constexpr auto a = sizeof(AreaTrait);
    // constexpr auto b = sizeof(AddTrait);
}

void traits_tb(){
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&hal::usart2);

    Rectangle rect = {1,1};
    Circle circle = {1};

    area(&rect);
    auto * ptr = &rect;
    // area(nullptr);
    area(ptr);
    area(&circle);

    DEBUG_PRINTLN("\r\nstart");
}

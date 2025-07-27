#include "core/debug/debug.hpp"

#include "core/math/real.hpp"
#include "core/clock/time.hpp"

#include "core/polymorphism/traits.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;



struct AreaTrait{
private: 
    template<typename T> 
    struct vtable_t {
        using Self = vtable_t<T>;
        static real_t area_dispatcher (const void* self ) {
            return (reinterpret_cast<const T *>(self))->area ( ); 
        };

        static constexpr real_t (* area) (const void* self ) = &Self::area_dispatcher;
    }; 

    struct vtable{
        real_t (* area) (const void* self );
    }; 

    struct body{
        public:
        inline real_t area ( ) const{
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

    real_t area() const { return radius_ * radius_ * real_t(PI);}

    friend OutputStream & operator<<(OutputStream & os, const Circle & circle) {
        return os << "{Circle: radius = " << circle.radius_ << "}";
    }
private:
    real_t radius_;
};


void area(AreaTrait obj){
    DEBUG_PRINTLN(obj->area());

    // constexpr auto a = sizeof(AreaTrait);
    // constexpr auto b = sizeof(AddTrait);
}

void traits_tb(){
    hal::uart2.init({576000});
    DEBUGGER.retarget(&hal::uart2);

    Rectangle rect = {1,1};
    Circle circle = {1};

    area(&rect);
    auto * ptr = &rect;
    // area(nullptr);
    area(ptr);
    area(&circle);

    DEBUG_PRINTLN("\r\nstart");
}

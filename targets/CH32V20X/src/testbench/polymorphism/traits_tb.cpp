#include "sys/debug/debug.hpp"
#include "sys/stream/StringStream.hpp"

#include "sys/math/real.hpp"
#include "sys/clock/time.hpp"

#include "sys/polymorphism/traits.hpp"

using namespace ymd;

// TRAIT_STRUCT(AddTrait,
// 	TRAIT_METHOD(void, add, int),
// 	// TRAIT_METHOD(void, add, real_t),
// 	TRAIT_METHOD(void, sub, int)
// )

// TRAIT_STRUCT(AreaTrait,
// 	TRAIT_METHOD(real_t, area)
// )
// template<typename T> 
// struct AreaTrait_Vtable_T {
//     using Self = AreaTrait_Vtable_T<T>;
//     real_t (* area) (void* self ) = &Self::static_area;
//     static real_t static_area (void* self ) {
//          return ((T*)self)->area ( ); 
//     }; 
// }; 

// struct AreaTrait_Vtable{
//     real_t (* area) (void* self );
// }; 

// struct AreaTrait{
//     void* self = nullptr; 
//     AreaTrait() = delete;
//     inline real_t area ( ) {
//         return vtable_-> area (_get_self() );
//     } 
    
//     template<typename T>
//     AreaTrait(T& t) : self(&t) { 
//         static AreaTrait_Vtable_T<T> impl;
//         vtable_ = (AreaTrait_Vtable*)(void*)&impl;
//     } 
    
//     private: inline void* _get_self() {
//             return self;
//         } 
//     AreaTrait_Vtable* vtable_;
// };

template<typename T> 
struct AreaTrait_Vtable_T {
    using Self = AreaTrait_Vtable_T<T>;
    real_t (* area) (const void* self ) = &Self::static_area;
    static real_t static_area (const void* self ) {
         return ((const T*)self)->area ( ); 
    };
}; 

struct AreaTrait_Vtable{
    real_t (* area) (const void* self );
}; 

struct AreaTrait{
public:
    AreaTrait() = delete;
    inline real_t area ( ) const{
        return vtable_-> area (_get_self() );
    } 
    
    template<typename T>
    AreaTrait(T * t) : self(&t) { 
        static AreaTrait_Vtable_T<T> impl;
        vtable_ = reinterpret_cast<AreaTrait_Vtable*>(&impl);
    }
    
private: 
    inline const void* _get_self() const{
        return self;
    } 

    const void* self = nullptr; 
    AreaTrait_Vtable* vtable_;
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
    DEBUG_PRINTLN(obj.area());

    // constexpr auto a = sizeof(AreaTrait);
    // constexpr auto b = sizeof(AddTrait);
}

void traits_tb(){
    hal::uart2.init(DEBUG_UART_BAUD);
    DEBUGGER.retarget(&hal::uart2);

    Rectangle rect = {1,1};
    Circle circle = {1};

    area(&rect);
    auto * ptr = &rect;
    // area(nullptr);
    area(ptr);
    area(&circle);

    DEBUG_PRINTLN("\r\nstart")
}

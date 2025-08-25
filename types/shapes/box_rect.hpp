#pragma once

#include "types/shapes/prelude.hpp"
#include "types/regions/rect2.hpp"

namespace ymd{


template<typename T>
struct BoxRect final{
    T width;
    T height;

    // constexpr auto to_cache() const {
    //     return Cache{
    //         .half_width = width / 2,
    //         .half_height = height / 2,
    //     };
    // }

    constexpr auto to_bounding_box() const {
        return Rect2<T>{-width/2,-height/2, width, height};
    }
};

// struct Cache{
//     T half_width;
//     T half_height;

//     __fast_inline constexpr uint8_t color_from_point(const Vec2<T> offset) const {
//         return s_color_from_point(*this, offset);
//     }
// private:
//     __fast_inline static constexpr uint8_t s_color_from_point(const Cache & self, const Vec2<T> offset){
//         return 
//             ((abs(offset.x) - (self.half_width) <= 0)
//             and (abs(offset.y) - (self.half_height) <= 0)) ? 0xff : 0;
//         ;
//     }
// };

template<typename T>
struct BoundingBoxOf<Rect2<T>> {
    using Object = Rect2<T>;

    static constexpr auto to_bounding_box(const Object & obj){
        return obj;
    }
};

template<typename T>
struct is_placed_t<BoxRect<T>>:std::false_type{;};

template<typename T>
struct is_placed_t<Rect2<T>>:std::true_type{;};

template<typename T>
requires (std::is_integral_v<T>)
struct ScanLinesIterator<Rect2<T>>{
    using Shape = Rect2<T>;
    using Self = ScanLinesIterator<Shape>;
    static constexpr Self from(const Shape & shape){
        const auto ret = Self{};

        ret.x_range = shape.get_x_range();
        ret.y_stop = shape.y() + shape.h();
        ret.y_ = shape.y();

        return ret;
    }

    static constexpr Self from(const Shape & shape, const T y){
        const auto ret = Self{};

        ret.x_range = shape.get_x_range();
        ret.y_stop = shape.y() + shape.h();
        ret.y_ = y;

        return ret;
    }

    constexpr bool has_next(){
        return y_ < y_stop;
    }

    constexpr ScanLine next(){ 
        return ScanLine{x_range_, y_++};
    }
private:
    Range2u x_range_;
    T y_stop;
    T y_;
};


}
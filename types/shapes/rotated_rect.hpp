#pragma once

#include "types/shapes/prelude.hpp"
#include "types/regions/rect2.hpp"

namespace ymd{


template<typename T>
struct RotatedRect{
    T width;
    T height;
    T orientation;

    using Self = RotatedRect<T>;


    template<size_t I>
    requires ((0 <= I) and (I < 4))
    constexpr Vec2<T> get_vertice() const {
        switch(I){
            case 0: return {-width / 2, height / 2};
            case 1: return {width / 2, height / 2};
            case 2: return {-width / 2, -height / 2};
            case 3: return {width / 2, -height / 2};
            default: __builtin_unreachable();
        }
    }

    constexpr Rect2<T> bounding_box() const {
        auto & self = *this;
        const auto rot = Vec2<T>::from_idenity_rotation(self.orientation);
        const std::array<Vec2<T>, 4> points = {
            self.template get_vertice<0>().improduct(rot),
            self.template get_vertice<1>().improduct(rot),
            self.template get_vertice<2>().improduct(rot),
            self.template get_vertice<3>().improduct(rot)
        };

        return Rect2<T>::from_minimal_bounding_box(std::span(points));
    }
};

template<typename T>
struct CacheOf<RotatedRect<T>, bool>{
    using Object = RotatedRect<T>;
    using Self = CacheOf<Object, bool>;

    T half_width;
    T half_height;
    T s;
    T c;


    static constexpr Self from(const Object & obj){
        const auto [s,c] = sincos(obj.orientation);
        return Self{
            .half_width = obj.width / 2,
            .half_height = obj.height / 2,
            .s = s,
            .c = c
        };
    }

    __fast_inline constexpr uint8_t color_from_point(const Vec2<T> offset) const {
        return contains_point(*this, offset);
    }
private:
    __fast_inline static constexpr bool contains_point(
        const Self & self, const Vec2<T> offset
    ){
        // -s * p.x + c * p.y;
        // -c * p.x - s * p.y;
        return 
            ((abs(-self.s * offset.x + self.c * offset.y)
                <= self.half_height) and
            (abs(-self.c * offset.x - self.s * offset.y) 
                <= self.half_width));
        ;
    }
};


template<typename T>
struct BoundingBoxOf<RotatedRect<T>>{

    using Object = RotatedRect<T>;
    using Self =  BoundingBoxOf<Object>;

    static constexpr Rect2<T> bounding_box(const Object & obj){
        return obj.bounding_box();
    }
};

template<typename T>
struct is_placed_t<RotatedRect<T>>:std::false_type {};

}
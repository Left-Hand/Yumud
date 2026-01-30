#pragma once

#include "algebra/shapes/prelude.hpp"
#include "algebra/regions/rect2.hpp"

namespace ymd{


template<typename T>
struct RotatedRect{
    T width;
    T height;
    Angular<T> rotation;

    using Self = RotatedRect<T>;


    template<size_t I>
    requires ((0 <= I) and (I < 4))
    constexpr math::Vec2<T> get_vertice() const {
        switch(I){
            case 0: return {-width / 2, height / 2};
            case 1: return {width / 2, height / 2};
            case 2: return {width / 2, -height / 2};
            case 3: return {-width / 2, -height / 2};
            default: __builtin_unreachable();
        }
    }

    constexpr math::Rect2<T> bounding_box() const {
        auto & self = *this;
        const auto norm_vec = math::Vec2<T>::from_angle(self.rotation);
        const std::array<math::Vec2<T>, 4> points = {
            self.template get_vertice<0>().improduct(norm_vec),
            self.template get_vertice<1>().improduct(norm_vec),
            self.template get_vertice<2>().improduct(norm_vec),
            self.template get_vertice<3>().improduct(norm_vec)
        };

        return math::Rect2<T>::from_minimal_bounding_box(std::span(points));
    }
};

template<typename T>
struct PreComputedOf<RotatedRect<T>, bool>{
    using Object = RotatedRect<T>;
    using Self = PreComputedOf<Object, bool>;

    T half_width;
    T half_height;
    T s;
    T c;


    static constexpr Self from(const Object & obj){
        const auto [s,c] = obj.rotation.sincos();
        return Self{
            .half_width = obj.width / 2,
            .half_height = obj.height / 2,
            .s = static_cast<T>(s),
            .c = static_cast<T>(c)
        };
    }

    __fast_inline constexpr uint8_t color_from_point(const math::Vec2<T> offset) const {
        return contains_point(*this, offset) ? 0xff : 0x00;
    }
private:
    __fast_inline static constexpr bool contains_point(
        const Self & self, const math::Vec2<T> offset
    ){
        // -s * p.x + c * p.y;
        // -c * p.x - s * p.y;
        return 
            ((math::abs(-self.s * offset.x + self.c * offset.y)
                <= self.half_height) and
            (math::abs(-self.c * offset.x - self.s * offset.y) 
                <= self.half_width));
        ;
    }
};


template<typename T>
struct BoundingBoxOf<RotatedRect<T>>{

    using Object = RotatedRect<T>;
    using Self =  BoundingBoxOf<Object>;

    static constexpr math::Rect2<T> bounding_box(const Object & obj){
        return obj.bounding_box();
    }
};

template<typename T>
struct is_placed_t<RotatedRect<T>>:std::false_type {};

}
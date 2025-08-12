#pragma once

#include "types/regions/rect2.hpp"

namespace ymd{

template<typename T>
struct RotatedRect{
    T width;
    T height;
    T rotation;

    struct alignas(4) Cache{
        T half_width;
        T half_height;
        T s;
        T c;

        __fast_inline constexpr uint8_t color_from_point(const Vector2<T> offset) const {
            return s_color_from_point(*this, offset);
        }
    private:
        __fast_inline static constexpr uint8_t s_color_from_point(
            const Cache & self, const Vector2<T> offset){
            // -s * p.x + c * p.y;
            // -c * p.x - s * p.y;
            return 
                ((abs(-self.s * offset.x + self.c * offset.y)
                    <= self.half_height) and
                (abs(-self.c * offset.x - self.s * offset.y) 
                    <= self.half_width))
                    
                ? 0xff : 0
            ;
        }
    };

    constexpr auto to_cache() const {
        const auto [s,c] = sincos(rotation);
        return Cache{
            .half_width = width / 2,
            .half_height = height / 2,
            .s = s,
            .c = c
        };
    }

    constexpr Rect2<T> to_bounding_box() const {
        const auto rot = Vector2<T>::from_idenity_rotation(rotation);
        const std::array<Vector2<T>, 4> points = {
            get_raw_point<0>().improduct(rot),
            get_raw_point<1>().improduct(rot),
            get_raw_point<2>().improduct(rot),
            get_raw_point<3>().improduct(rot)
        };

        return Rect2<T>::from_minimal_bounding_box(std::span(points));
    }

    template<size_t I>
    requires ((0 <= I) and (I < 4))
    constexpr Vector2<T> get_raw_point() const {
        switch(I){
            case 0: return {-width / 2, height / 2};
            case 1: return {width / 2, height / 2};
            case 2: return {-width / 2, -height / 2};
            case 3: return {width / 2, -height / 2};
            default: __builtin_unreachable();
        }
    }
};

}
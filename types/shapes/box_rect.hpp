#pragma once

#include "types/shapes/prelude.hpp"
#include "types/regions/rect2.hpp"

namespace ymd{


template<typename T>
struct BoxRect final{
    T width;
    T height;

    struct Cache{
        T half_width;
        T half_height;

        __fast_inline constexpr uint8_t color_from_point(const Vec2<T> offset) const {
            return s_color_from_point(*this, offset);
        }
    private:
        __fast_inline static constexpr uint8_t s_color_from_point(const Cache & self, const Vec2<T> offset){
            return 
                ((abs(offset.x) - (self.half_width) <= 0)
                and (abs(offset.y) - (self.half_height) <= 0)) ? 0xff : 0;
            ;
        }
    };

    constexpr auto to_cache() const {
        return Cache{
            .half_width = width / 2,
            .half_height = height / 2,
        };
    }

    constexpr auto to_bounding_box() const {
        return Rect2<T>{-width/2,-height/2, width, height};
    }
};

template<typename T>
struct is_placed_t<BoxRect<T>>:std::false_type{;};


}
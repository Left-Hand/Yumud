#pragma once

#include "types/shapes/prelude.hpp"

namespace ymd{



template<typename T>
struct RoundedRectangle{
    Vec2<T> size;
    CornerRadii<T> corners;

    constexpr Rect2<T> bounding_box() const{
        return Rect2<T>::from_size(size);
    }
};

template<typename T>
struct BoundingBoxOf<RoundedRectangle<T>>{
    using Object = RoundedRectangle<T>;

    static constexpr auto bounding_box(const Object & obj) {
        return obj.bounding_box();
    }
};


}
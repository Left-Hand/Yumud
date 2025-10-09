#pragma once

#include "vector2.hpp"

namespace ymd{


template<typename T>
struct Polar{
    T amplitude;
    Angle<T> phase;

    [[nodiscard]] constexpr Vec2<T> to_vec2() const {
        return Vec2<T>::from_angle_and_length(phase, amplitude);
    }
};


}
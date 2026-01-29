#pragma once

#include "algebra/vectors/vec2.hpp"

namespace ymd::math{

template<typename T>
struct [[nodiscard]] Twist2 final{
    Vec2<T> linear;
    Angular<T> angular;
};

}
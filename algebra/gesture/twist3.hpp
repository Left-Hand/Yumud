#pragma once

#include "algebra/vectors/math::Vec3.hpp"

namespace ymd::math{

template<typename T>
struct [[nodiscard]] Twist3 final{
    math::Vec3<T> linear;
    math::Vec3<T> angular;
};

}
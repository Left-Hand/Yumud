#pragma once

#include "algebra/vectors/Vec3.hpp"

namespace ymd{

template<typename T>
struct [[nodiscard]] Twist3 final{
    Vec3<T> linear;
    Vec3<T> angular;
};

}
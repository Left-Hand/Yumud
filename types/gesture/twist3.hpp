#pragma once

#include "types/vectors/Vec3.hpp"

namespace ymd{

template<typename T>
class Twist3 final{
    Vec3<T> linear;
    Vec3<T> angular;
};

}
#pragma once

#include "types/vectors/vector2/Vector3.hpp"

namespace ymd{

template<typename T>
class Twist3 final{
    Vector3<T> linear;
    Vector3<T> angular;
};

}
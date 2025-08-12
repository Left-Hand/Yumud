#pragma once

#include "types/vectors/Vector3.hpp"

namespace ymd{

template<typename T>
class Twist3 final{
    Vector3<T> linear;
    Vector3<T> angular;
};

}
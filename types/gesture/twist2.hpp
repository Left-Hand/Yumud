#pragma once

#include "types/vectors/vector2.hpp"

namespace ymd{

template<typename T>
class Twist2 final{
    Vec2<T> linear;
    T angular;
};

}
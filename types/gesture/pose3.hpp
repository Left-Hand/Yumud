#pragma once

#include "types/vectors/Vector3.hpp"
#include "types/vectors/quat.hpp"

namespace ymd{

template<typename T>
class Pose3 final{
    Vector3<T> position;
    Quat<T> orientation;

    // [[nodiscard]] constexpr Pose3<T> forward_move(const T length) const {

    // }
};

}
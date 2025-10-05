#pragma once

#include "types/vectors/vector3.hpp"
#include "core/utils/angle.hpp"

namespace ymd {

enum class EulerAnglePolicy {
    XYZ,
    ZYX,
    YXZ
};

template <arithmetic T, EulerAnglePolicy P>
struct _EulerAngleStorage;

template <arithmetic T>
struct _EulerAngleStorage<T, EulerAnglePolicy::XYZ> {
    Angle<T> x, y, z;
};

template <arithmetic T>
struct _EulerAngleStorage<T, EulerAnglePolicy::ZYX> {
    Angle<T> z, y, x;
};

template <arithmetic T>
struct _EulerAngleStorage<T, EulerAnglePolicy::YXZ> {
    Angle<T> y, x, z;
};

// Main EulerAngles template
template <arithmetic T, EulerAnglePolicy P>
using EulerAngles = _EulerAngleStorage<T, P>;

template <arithmetic T, EulerAnglePolicy P>
__fast_inline OutputStream & operator<<(OutputStream & os, const EulerAngles<T, P> & value){
    return os << os.brackets<'('>()
        << value.x << os.splitter()
        << value.y << os.splitter()
        << value.z << os.brackets<')'>();
}

}
#pragma once

#include "types/vectors/vector3.hpp"
#include "primitive/angle.hpp"

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

    using Self = _EulerAngleStorage<T, EulerAnglePolicy::XYZ>;
    [[nodiscard]] static constexpr Self from_xyz(
        const Angle<T> x, 
        const Angle<T> y,
        const Angle<T> z
    ){
        return {.x = x, .y = y, .z = z};
    }

    [[nodiscard]] constexpr Self to_xyz() const {
        return *this;
    }
};

template <arithmetic T>
struct _EulerAngleStorage<T, EulerAnglePolicy::ZYX> {
    Angle<T> z, y, x;

    using Self = _EulerAngleStorage<T, EulerAnglePolicy::ZYX>;
    [[nodiscard]] static constexpr Self from_xyz(
        const Angle<T> x, 
        const Angle<T> y,
        const Angle<T> z
    ){
        return {.z = z, .y = y, .x = x};
    }

    [[nodiscard]] constexpr _EulerAngleStorage<T, EulerAnglePolicy::XYZ> to_xyz() const {
        return {.x = x, .y = y, .z = z};
    }
};

template <arithmetic T>
struct _EulerAngleStorage<T, EulerAnglePolicy::YXZ> {
    Angle<T> y, x, z;

    using Self = _EulerAngleStorage<T, EulerAnglePolicy::YXZ>;

    [[nodiscard]] static constexpr Self from_xyz(
        const Angle<T> x, 
        const Angle<T> y,
        const Angle<T> z
    ){
        return {.y = y, .x = x, .z = z};
    }

    [[nodiscard]] constexpr _EulerAngleStorage<T, EulerAnglePolicy::XYZ> to_xyz() const {
        return {.x = x, .y = y, .z = z};
    }
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
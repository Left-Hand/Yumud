#pragma once

#include "types/vector3/vector3.hpp"

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
    // union{T x; T roll;};
    // union{T y; T yaw;};
    // union{T z; T pitch;};
    T x, y, z;
};

template <arithmetic T>
struct _EulerAngleStorage<T, EulerAnglePolicy::ZYX> {
    // union{T z; T pitch;};
    // union{T y; T yaw;};
    // union{T x; T roll;};
    T z, y, x;
};

template <arithmetic T>
struct _EulerAngleStorage<T, EulerAnglePolicy::YXZ> {
    // union{T y; T yaw;};
    // union{T x; T roll;};
    // union{T z; T pitch;};
    T y, x, z;
};

// Main EulerAngle_t template
template <arithmetic T, EulerAnglePolicy P>
using EulerAngle_t = _EulerAngleStorage<T, P>;
// struct EulerAngle_t : public _EulerAngleStorage<T, P> {
    // using _EulerAngleStorage<T, P>::_EulerAngleStorage<T, P>;
    // // Default constructor
    // constexpr EulerAngle_t() = default;
    // // Copy constructor
    // constexpr EulerAngle_t(const EulerAngle_t& other) = default;
    
    // // Move constructor
    // constexpr EulerAngle_t(EulerAngle_t&& other) = default;
    
    // // Assignment operators
    // constexpr EulerAngle_t& operator=(const EulerAngle_t& other) = default;
    // constexpr EulerAngle_t& operator=(EulerAngle_t&& other) = default;
// };
template <arithmetic T, EulerAnglePolicy P>
__fast_inline OutputStream & operator<<(OutputStream & os, const EulerAngle_t<T, P> & value){
    return os << os.brackets<'('>()
        << value.x << os.splitter()
        << value.y << os.splitter()
        << value.z << os.brackets<')'>();
}

}
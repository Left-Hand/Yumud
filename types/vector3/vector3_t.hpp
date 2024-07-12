#ifndef __VECTOR3_HPP__

#define __VECTOR3_HPP__

#include "sys/core/platform.h"
#include "../string/String.hpp"
#include "types/real.hpp"
#include <type_traits>

template <typename real>
requires std::is_arithmetic_v<real>
struct Vector3_t{
public:
    real x = real(0);
    real y = real(0);
    real z = real(0);

    Vector3_t() = default;

    template<typename U>
    Vector3_t(const Vector3_t<U>& v) : x(v.x), y(v.y), z(v.z) {;}

    template<typename U>
    Vector3_t(const U & _x, const U & _y, const U & _z): x(_x), y(_y), z(_z){;}

    real & operator [](const int & index) { return *(&this->x + index); }
    template<typename U>
    Vector3_t& operator=(const Vector3_t<U>& v) {
        x = static_cast<real>(v.x);
        y = static_cast<real>(v.y);
        z = static_cast<real>(v.z);
        return *this;
    };

    template<typename U>
    Vector3_t & operator += (const Vector3_t<U>& v) {
        x += static_cast<real>(v.x);
        y += static_cast<real>(v.y);
        z += static_cast<real>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator -= (const Vector3_t<U>& v) {
        x -= static_cast<real>(v.x);
        y -= static_cast<real>(v.y);
        z -= static_cast<real>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator *= (const Vector3_t<U>& v) {
        x *= static_cast<real>(v.x);
        y *= static_cast<real>(v.y);
        z *= static_cast<real>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator *= (const U & _v){
        real v = static_cast<real>(_v);
        x *= v;
        y *= v;
        z *= v;
        return *this;
    }

    template<typename U>
    Vector3_t & operator /= (const Vector3_t<U>& v) {
        x /= static_cast<real>(v.x);
        y /= static_cast<real>(v.y);
        z /= static_cast<real>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator /= (const U & _v){
        real v = static_cast<real>(_v);
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    template<typename U>
    real dot(const Vector3_t<U> &v) const{
        real ret = real(0);
        ret += x * v.x;
        ret += y * v.y;
        ret += z * v.z;
        return ret;
    }

    template<typename U>
    Vector3_t cross(const Vector3_t<U> &u) const{
        Vector3_t v = u;
        return Vector3_t(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }


    real length() const{
        return sqrt(x * x + y * y + z * z);
    }

    real length_squared() const{
        return x * x + y * y + z * z;
    }

    void normalize() {
        real_t lengthsq = length_squared();
        if (lengthsq == 0) {
            x = y = z = 0;
        } else {
            real_t length = sqrt(lengthsq);
            x /= length;
            y /= length;
            z /= length;
        }
    }

    Vector3_t normalized() const {
        Vector3_t v = *this;
        v.normalize();
        return v;
    }

    // __no_inline String toString(unsigned char decimalPlaces = 2){
    //     return (String('(') + toString(x, decimalPlaces) +
    //             String(", ") + toString(y, decimalPlaces) +
    //             String(", ") + toString(z, decimalPlaces) +
    //             String(')'));
    // }

};

typedef Vector3_t<real_t> Vector3;

#endif
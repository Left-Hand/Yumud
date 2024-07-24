#ifndef __VECTOR3_HPP__

#define __VECTOR3_HPP__

#include "../sys/core/platform.h"
#include "../string/String.hpp"
#include "../types/real.hpp"
#include <type_traits>

template <arithmetic T>
struct Vector3_t{
public:
    T x = T(0);
    T y = T(0);
    T z = T(0);

    Vector3_t() = default;

    template<arithmetic U>
    Vector3_t(const Vector3_t<U>& v) : x(v.x), y(v.y), z(v.z) {;}

    // template<arithmetic U>
    Vector3_t(const auto _x, const auto _y, const auto _z): x(_x), y(_y), z(_z){;}

    template<arithmetic U, arithmetic V, arithmetic Z>
    __fast_inline constexpr Vector3_t(const std::tuple<U, V, Z> & v) : x(std::get<0>(v)), y(std::get<1>(v)), z(std::get<2>(v)){;}

    T & operator [](const int & index) { return *(&this->x + index); }
    template<arithmetic U>
    Vector3_t& operator=(const Vector3_t<U>& v) {
        x = static_cast<T>(v.x);
        y = static_cast<T>(v.y);
        z = static_cast<T>(v.z);
        return *this;
    };

    template<arithmetic U>
    Vector3_t & operator += (const Vector3_t<U>& v) {
        x += static_cast<T>(v.x);
        y += static_cast<T>(v.y);
        z += static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    Vector3_t & operator -= (const Vector3_t<U>& v) {
        x -= static_cast<T>(v.x);
        y -= static_cast<T>(v.y);
        z -= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    Vector3_t & operator *= (const Vector3_t<U>& v) {
        x *= static_cast<T>(v.x);
        y *= static_cast<T>(v.y);
        z *= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    Vector3_t & operator *= (const U & _v){
        T v = static_cast<T>(_v);
        x *= v;
        y *= v;
        z *= v;
        return *this;
    }

    template<arithmetic U>
    Vector3_t & operator /= (const Vector3_t<U>& v) {
        x /= static_cast<T>(v.x);
        y /= static_cast<T>(v.y);
        z /= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    Vector3_t & operator /= (const U & _v){
        T v = static_cast<T>(_v);
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    template<arithmetic U>
    __fast_inline_constexpr Vector3_t operator *(const U & _v) const{
        Vector3_t other = *this;
        other *= _v;
        return other;
    }

    template<arithmetic U>
    __fast_inline_constexpr Vector3_t operator /(const U & _v) const{
        Vector3_t other = *this;
        other /= _v;
        return other;
    }

    template<arithmetic U>
    __fast_inline_constexpr Vector3_t operator +(const Vector3_t<U>& other) const {
        Vector3_t ret = other;
        return ret += *this;
    }

    template<arithmetic U>
    __fast_inline_constexpr Vector3_t operator -(const Vector3_t<U>& other) const {
        Vector3_t ret = other;
        return ret -= *this;
    }


    template<arithmetic U>
    constexpr Vector3_t<T> clampmin(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l < length ? *this * length / l : *this);
    }

    template<arithmetic U>
    constexpr Vector3_t<T> clampmax(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l > length ? *this * length / l : *this);
    }


    template<arithmetic U>
    T dot(const Vector3_t<U> &v) const{
        T ret = T(0);
        ret += x * v.x;
        ret += y * v.y;
        ret += z * v.z;
        return ret;
    }

    template<arithmetic U>
    Vector3_t cross(const Vector3_t<U> &u) const{
        Vector3_t v = u;
        return Vector3_t(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }


    T length() const{
        return sqrt(x * x + y * y + z * z);
    }

    T length_squared() const{
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
}__packed;

#include "vector3_t.tpp"

using Vector3 = Vector3_t<real_t>;
using Vector3i = Vector3_t<int>;

__fast_inline OutputStream & operator<<(OutputStream & os, const Vector3_t<auto> & value){
    return os << '(' << value.x << ',' << value.y << ',' << value.z << ')';
}

#endif
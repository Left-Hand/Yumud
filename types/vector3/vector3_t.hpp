#ifndef __VECTOR3_HPP__

#define __VECTOR3_HPP__

#include "src/platform.h"
#include "../string/String.hpp"
#include "../real.hpp"
#include <type_traits>

template <typename T>
struct Vector3_t{
public:
    T x = T(0);
    T y = T(0);
    T z = T(0);


    template<typename U>
    Vector3_t(const Vector3_t<U>& v) : x(v.x), y(v.y), z(v.z) {;}

    template<typename U>
    Vector3_t(const U & _x, const U & _y, const U & _z): x(_x), y(_y), z(_z){;}

    T & operator [](const int & index) { return *(&this->x + index); }
    template<typename U>
    Vector3_t& operator=(const Vector3_t<U>& v) {
        x = static_cast<T>(v.x);
        y = static_cast<T>(v.y);
        z = static_cast<T>(v.z);
        return *this;
    };

    template<typename U>
    Vector3_t & operator += (const Vector3_t<U>& v) {
        x += static_cast<T>(v.x);
        y += static_cast<T>(v.y);
        z += static_cast<T>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator -= (const Vector3_t<U>& v) {
        x -= static_cast<T>(v.x);
        y -= static_cast<T>(v.y);
        z -= static_cast<T>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator *= (const Vector3_t<U>& v) {
        x *= static_cast<T>(v.x);
        y *= static_cast<T>(v.y);
        z *= static_cast<T>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator *= (const U & _v){
        T v = static_cast<T>(_v);
        x *= v;
        y *= v;
        z *= v;
        return *this;
    }

    template<typename U>
    Vector3_t & operator /= (const Vector3_t<U>& v) {
        x /= static_cast<T>(v.x);
        y /= static_cast<T>(v.y);
        z /= static_cast<T>(v.z);
        return *this;
    }

    template<typename U>
    Vector3_t & operator /= (const U & _v){
        T v = static_cast<T>(_v);
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    template<typename U>
    T dot(const Vector3_t<U> &v) const{
        T ret = T(0);
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


    T length() const{
        return sqrt(x * x + y * y + z * z);
    }
};

typedef Vector3_t<real_t> Vector3;

#endif
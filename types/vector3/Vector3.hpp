#pragma once

/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/* Copyright (c) 2024  Rstr1aN / Yumud                                    */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/*                                                                        */
/* Note: This file has been modified by Rstr1aN / Yumud.                  */
/**************************************************************************/

#include "types/vector2/vector2.hpp"

namespace ymd{
template <arithmetic T>
struct Vector3_t{
public:
    T x;
    T y;
    T z;


    scexpr Vector3_t<T> ZERO = Vector3_t<T>(0, 0, 0);
    scexpr Vector3_t<T> ONE = Vector3_t<T>(1, 1, 1);

    scexpr Vector3_t<T> UP = Vector3_t<T>(0, 0, 1);
    scexpr Vector3_t<T> DOWN = Vector3_t<T>(0, 0, -1);
    
    scexpr Vector3_t<T> INF = Vector3_t<T>(INFINITY, INFINITY, INFINITY);

    
    __fast_inline constexpr Vector3_t(){;}

    __fast_inline constexpr Vector3_t(const Vector3_t<arithmetic auto>& v) : x(v.x), y(v.y), z(v.z) {;}

    __fast_inline constexpr Vector3_t(const Vector2_t<arithmetic auto>& v, const arithmetic auto z_) : x(v.x), y(v.y), z(z_) {;}

    __fast_inline constexpr Vector3_t(const T & _x, const T & _y, const T & _z): x((_x)), y((_y)), z((_z)){;}

    __fast_inline static constexpr Vector3_t from_ones(const T & _x){return Vector3_t<T>(_x, _x, _x);}

    template<arithmetic U = T>
    __fast_inline constexpr Vector3_t(const std::tuple<U, U, U> & v) : x(std::get<0>(v)), y(std::get<1>(v)), z(std::get<2>(v)){;}

    T & operator [](const size_t idx) { return (&x)[idx];}

    const T & operator [](const size_t idx) const {return (&x)[idx];}

    template<arithmetic U>
    Vector3_t& operator=(const Vector3_t<U>& v) {
        x = static_cast<T>(v.x);
        y = static_cast<T>(v.y);
        z = static_cast<T>(v.z);
        return *this;
    };

    template<arithmetic U>
    __fast_inline
    Vector3_t & operator += (const Vector3_t<U>& v) {
        x = static_cast<T>(x + static_cast<T>(v.x));
        y = static_cast<T>(y + static_cast<T>(v.y));
        z = static_cast<T>(z + static_cast<T>(v.z));
        return *this;
    }
    
    template<arithmetic U>
    __fast_inline
    Vector3_t & operator -= (const Vector3_t<U>& v) {
        x -= static_cast<T>(v.x);
        y -= static_cast<T>(v.y);
        z -= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    __fast_inline
    Vector3_t & operator *= (const Vector3_t<U>& v) {
        x *= static_cast<T>(v.x);
        y *= static_cast<T>(v.y);
        z *= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr Vector3_t<T> increase_x(const U & v){
        return {x + v, y, z};
    }

    template<arithmetic U>
    __fast_inline constexpr Vector3_t<T> increase_y(const U & v){
        return {x, y + v, z};
    }

    template<arithmetic U>
    __fast_inline constexpr Vector3_t<T> increase_z(const U & v){
        return {x, y, z + v};
    }

    template<arithmetic U>
    __fast_inline
    constexpr Vector3_t & operator *= (const U & _v){
        T v = static_cast<T>(_v);
        x = x * v;
        y = y * v;
        z = z * v;
        return *this;
    }

    template<arithmetic U>
    constexpr Vector3_t & operator /= (const Vector3_t<U>& v) {
        x /= static_cast<T>(v.x);
        y /= static_cast<T>(v.y);
        z /= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    constexpr Vector3_t & operator /= (const U & _v){
        T v = static_cast<T>(_v);
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector3_t operator *(const U & _v) const{
        Vector3_t other = *this;
        other *= _v;
        return other;
    }

    [[nodiscard]] __fast_inline constexpr Vector3_t operator *(const Vector3_t<arithmetic auto> & _v) const{
        Vector3_t other = *this;
        other *= _v;
        return other;
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector3_t operator /(const U & _v) const{
        Vector3_t other = *this;
        other /= _v;
        return other;
    }

    [[nodiscard]] __fast_inline constexpr Vector3_t operator /(const Vector3_t<arithmetic auto> & _v) const{
        Vector3_t other = *this;
        other /= _v;
        return other;
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector3_t operator +(const Vector3_t<U>& other) const {
        Vector3_t ret = other;
        return ret += *this;
    }

    [[nodiscard]] __fast_inline constexpr Vector3_t abs() const{
        return Vector3_t{
            ABS(x),
            ABS(y),
            ABS(z)
        };
    }

	[[nodiscard]] Vector3_t minf(arithmetic auto p_scalar) const {
		return Vector3_t(MIN(x, static_cast<T>(p_scalar)), MIN(y, static_cast<T>(p_scalar)), MIN(z, static_cast<T>(p_scalar)));
	}

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector3_t operator -(const Vector3_t<U>& other) const {
        return Vector3_t{
            x - static_cast<T>(other.x),
            y - static_cast<T>(other.y),
            z - static_cast<T>(other.z)
        };
    }
    
    [[nodiscard]] __fast_inline constexpr Vector3_t operator -() const {
        return Vector3_t{-x,-y,-z};
    }


    template<arithmetic U>
    [[nodiscard]] constexpr Vector3_t<T> clampmin(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l < length ? *this * length / l : *this);
    }

    template<arithmetic U>
    [[nodiscard]] constexpr Vector3_t<T> clampmax(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l > length ? *this * length / l : *this);
    }

    constexpr Vector2_t<T> xy() const{
        return {x,y};
    }


    template<arithmetic U>
    constexpr T dot(const Vector3_t<U> &v) const{
        return x * static_cast<T>(v.x) + y * static_cast<T>(v.y) + z * static_cast<T>(v.z);
    }

    template<arithmetic U>
    __fast_inline
    constexpr Vector3_t cross(const Vector3_t<U> &u) const{
        return Vector3_t(
            static_cast<T>(y * static_cast<T>(u.z) - z * static_cast<T>(u.y)),
            static_cast<T>(z * static_cast<T>(u.x) - x * static_cast<T>(u.z)), 
            static_cast<T>(x * static_cast<T>(u.y) - y * static_cast<T>(u.x))
        );
    }


    constexpr T length() const{
        return sqrt(x * x + y * y + z * z);
    }

    __fast_inline
    constexpr T length_squared() const{
        return x * x + y * y + z * z;
    }

    constexpr void normalize() {
        T lengthsq = length_squared();
        if (unlikely(lengthsq == 0)) {
            x = y = z = 0;
        } else{
            T inv_len = isqrt(lengthsq);
            x *= inv_len;
            y *= inv_len;
            z *= inv_len;
        }
    }

    constexpr Vector3_t normalized() const {
        Vector3_t v = *this;
        v.normalize();
        return v;
    }

    constexpr operator bool() const {
        return bool(x) or bool(y) or bool(z);
    }

    static bool sort_by_x(const Vector3_t & a, const Vector3_t & b){
        return a.x < b.x;
    };

    static bool sort_by_y(const Vector3_t & a, const Vector3_t & b){
        return a.y < b.y;
    };

    static bool sort_by_z(const Vector3_t & a, const Vector3_t & b){
        return a.y < b.y;
    };

    static bool sort_by_length(const Vector3_t & a, const Vector3_t & b){
        return a.length_squared() < b.length_square();
    };

};

template<arithmetic T>
__fast_inline constexpr Vector3_t<T> operator*(const arithmetic auto & n, const Vector3_t<T> & vec){
    return vec * n;
}


__fast_inline constexpr auto lerp(const Vector3_t<arithmetic auto> & a, const Vector3_t<arithmetic auto> & b, const arithmetic auto & t){
    return a + (b - a) * t;
}

__fast_inline constexpr auto distance(const Vector3_t<arithmetic auto> & a, const Vector3_t<arithmetic auto> & b){
    return (a - b).length();
}

__fast_inline constexpr auto normal(const Vector3_t<arithmetic auto> & from, const Vector3_t<arithmetic auto> & to){
    return (to - from).normalized();
}

__fast_inline OutputStream & operator<<(OutputStream & os, const Vector3_t<auto> & value){
    return os << os.brackets<'('>() << value.x << os.splitter() << value.y << os.splitter() << value.z << os.brackets<')'>();
}


// using Vector3_t<real_t> = Vector3_t<real_t>;

template<arithmetic T>
Vector3_t() -> Vector3_t<T>;
}


#include "vector3.tpp"
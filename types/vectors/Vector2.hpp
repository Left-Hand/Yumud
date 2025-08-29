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


#include "core/stream/ostream.hpp"
#include "core/math/real.hpp"
#include "core/utils/angle.hpp"

namespace ymd{

template<arithmetic T>
struct Rect2;

template<arithmetic T>
struct Vec2{
    static constexpr Vec2<T> ZERO = Vec2<T>(0, 0);
    static constexpr Vec2<T> ONE = Vec2<T>(1, 1);
    static constexpr Vec2<T> NEG_ONE = Vec2<T>(-1, -1);

    static constexpr Vec2<T> INF = Vec2<T>(
        std::numeric_limits<T>::max(), 
        std::numeric_limits<T>::max()
    );
    static constexpr Vec2<T> NEG_INF = Vec2<T>(
        std::numeric_limits<T>::min(), 
        std::numeric_limits<T>::min()
    );

    static constexpr Vec2<T> LEFT = Vec2<T>(-1, 0);
    static constexpr Vec2<T> RIGHT = Vec2<T>(1, 0);

    static constexpr Vec2<T> X = Vec2<T>(1, 0);
    static constexpr Vec2<T> Y = Vec2<T>(0, 1);
    static constexpr Vec2<T> NEG_X = Vec2<T>(-1, 0);
    static constexpr Vec2<T> NEG_Y = Vec2<T>(0, -1);
    static constexpr Vec2<T> AXES = Vec2<T>(0, 1);

    static constexpr Vec2<T> UP = Vec2<T>(0, 1);
    static constexpr Vec2<T> DOWN = Vec2<T>(0, -1);

    static constexpr Vec2<T> LEFT_UP = Vec2<T>(-1, 1);
    static constexpr Vec2<T> RIGHT_UP = Vec2<T>(1, 1);
    static constexpr Vec2<T> LEFT_DOWN = Vec2<T>(-1, -1);
    static constexpr Vec2<T> RIGHT_DOWN = Vec2<T>(1, -1);


    T x;
    T y;
    

    [[nodiscard]] constexpr Vec2(const T _x, const T _y): 
        x(T(_x)), y(T(_y)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Vec2(const std::tuple<U, U> & v) : 
        x(std::get<0>(v)), y(std::get<1>(v)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Vec2(const Vec2<U> & _v) : 
        x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    template<typename U>
    [[nodiscard]] __fast_inline static constexpr Vec2<T> from_angle(
        const Angle<U> angle
    ){
        static_assert(not std::is_integral_v<U>);
        static_assert(not std::is_integral_v<T>);

        const auto [s,c] = angle.sincos();
        return {static_cast<T>(c), static_cast<T>(s)};
    }

    template<typename U>
    [[nodiscard]] __fast_inline static constexpr Vec2<T> from_angle_and_length(
        const Angle<U> angle, const T length){
        const auto [s,c] = angle.sincos();
        return {static_cast<T>(c * length), static_cast<T>(s * length)};
    }

    [[nodiscard]] constexpr T & operator [](const size_t index) { 
        return *(&this->x + index);
    }

    [[nodiscard]] constexpr const T & operator [](const size_t index) const {
        return *(&this->x + index);
    }


    [[nodiscard]] static constexpr bool sort_by_x(const Vec2 & a, const Vec2 & b){
        return a.x < b.x;
    };

    [[nodiscard]] static constexpr bool sort_by_y(const Vec2 & a, const Vec2 & b){
        return a.y < b.y;
    };

    [[nodiscard]] static constexpr bool sort_by_length(const Vec2 & a, const Vec2 & b){
        return a.length_squared() < b.length_squared();
    };

    [[nodiscard]] static constexpr bool sort_by_angle(const Vec2 & a, const Vec2 & b){
        return a.cross(b) > 0;
    };

    [[nodiscard]] constexpr Vec2<T> normalized() const;
    [[nodiscard]] constexpr T cross(const Vec2<T> & other) const;

    template<typename U>
    [[nodiscard]] __fast_inline constexpr __attribute__((const)) bool 
    is_clockwise_to(const Vec2<U> & other) const {
        return (x*other.y > y*other.x);
    }
    
    template<typename U>
    [[nodiscard]] __fast_inline constexpr __attribute__((const)) bool 
    is_counter_clockwise_to(const Vec2<U> & other) const{
        return (x*other.y < y*other.x);
    }

    [[nodiscard]] constexpr T dot(const Vec2<T> & other) const;
    [[nodiscard]] constexpr Vec2<T> improduct(const Vec2<T> & b) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> rotated(const Angle<T> angle)const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> abs() const;



    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vec2<T> increase_x(const U & v){
        return {x + v, y};
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vec2<T> increase_y(const U & v){
        return {x, y + v};
    }

    [[nodiscard]] constexpr Angle<T> angle() const {
            return Angle<T>::from_radians(atan2(y, x));}
	[[nodiscard]] constexpr Angle<T> angle_between(const Vec2<T> & b) const {
        const auto & a = *this;
        // const T cross_z = a.x * b.y - a.y * b.x;
        // // 点积（cosθ）
        // const T dot = a.x * b.x + a.y * b.y;
        // if(ymd::is_equal_approx(dot, T(0))){
        //     return (dot >= 0) ? T(0) : T(PI);
        // }
        // return atan2(cross_z, dot);

        const T angle_a = atan2(a.y, a.x);
        const T angle_b = atan2(b.y, b.x);
        T diff = angle_b - angle_a;
        
        // 规范化到 [-π, π]
        while (diff > T(PI)) diff -= T(2 * PI);
        while (diff <= -T(PI)) diff += T(2 * PI);
        
        return Angle<T>::from_radians(diff);
    }

    [[nodiscard]] constexpr T aspect() const {return (!!y) ? x/y : T(0);}
    [[nodiscard]] constexpr Vec2<T> bounce(const Vec2<T> & n) const;

    template<typename U = T>
    [[nodiscard]] constexpr Vec2<U> ceil() const{
        return Vec2<U>{ceil_cast<U>(x), ceil_cast<U>(y)};
    }

    template<typename U = T>

    [[nodiscard]] constexpr Vec2<U> floor() const{
        return Vec2<U>{floor_cast<U>(x), floor_cast<U>(y)};
    }

    template<typename U = T>
    [[nodiscard]] constexpr Vec2<U> round() const{
        return Vec2<U>{round_cast<U>(x), round_cast<U>(y)};
    }

    template<arithmetic U>
    [[nodiscard]] constexpr Vec2<T> clampmin(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l < length ? *this * length / l : *this);
    }

    template<arithmetic U>
    [[nodiscard]] constexpr Vec2<T> clampmax(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l >= length ? (this->normalized() * length) : *this);
    }

    [[nodiscard]] constexpr Vec2<T> clamp(const T & _min, const T & _max) const;

    [[nodiscard]] constexpr Vec2<T> dir_to(const Vec2<T> & b) const;
    [[nodiscard]] constexpr T dist_to(const Vec2<T> & b) const;
    [[nodiscard]] constexpr T dist_squared_to(const Vec2<T> & b) const;

    [[nodiscard]] constexpr bool is_equal_approx(const Vec2<T> & v) const;
    [[nodiscard]] constexpr T manhattan_distance()const{
        return ABS(x) + ABS(y);
    }


    [[nodiscard]] constexpr bool has_point(const Vec2<auto> & _v)const;
    [[nodiscard]] constexpr bool is_normalized() const {return (fabs(x*x + y*y + T(-1)) <= T(CMP_EPSILON));}
    [[nodiscard]] constexpr T length() const {
        return mag(x,y);
    }

    [[nodiscard]] constexpr T inv_length() const {
        return imag(x,y);
    }
    
    [[nodiscard]] constexpr T length_squared() const {return (x*x + y*y);}
    
    [[nodiscard]] __fast_inline constexpr Vec2<T> lerp(const Vec2<T> & b, const T t) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> move_toward(const Vec2<T> & to, const T delta) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> midpoint_with(const Vec2<T> other) const 
        {return {(this->x + other.x) / 2, (this->y + other.y) / 2};}

    [[nodiscard]] __fast_inline constexpr Vec2<T> posmod(const T & mod) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> posmodv(const Vec2<T> & modv) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> project(const Vec2<T> & b) const;
    [[nodiscard]] __fast_inline constexpr T project(const T & rad) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> reflect(const Vec2<T> & n) const;

    [[nodiscard]] __fast_inline constexpr Vec2<T> sign() const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> slerp(const Vec2<T> & b, const T t) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> slide(const Vec2<T>  & n) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> snapped(const Vec2<T> & by) const;
    [[nodiscard]] __fast_inline constexpr Vec2<T> cw() const {return Vec2<T>(-y, x);}
    [[nodiscard]] __fast_inline constexpr Vec2<T> ccw() const {return Vec2<T>(y, -x);}

    [[nodiscard]] __fast_inline constexpr Vec2<T> flip_y() const {
        static_assert(std::is_signed_v<T>);
        return {x,static_cast<T>(-y)};
    }
    
    [[nodiscard]] __fast_inline constexpr Vec2<T> flip_x() const {
        static_assert(std::is_signed_v<T>);
        return {static_cast<T>(-x),y};
    }

    [[nodiscard]] __fast_inline constexpr Vec2<T> swap_xy() const {return {y,x};}

    __fast_inline constexpr Vec2<T> & operator=(const Vec2<auto> & b){
        x = static_cast<T>(b.x);
        y = static_cast<T>(b.y);
        return *this;
    }


    __fast_inline constexpr Vec2<T> & operator+=(const Vec2<auto> & b){
        x += static_cast<T>(b.x);
        y += static_cast<T>(b.y);
        return *this;
    }

    __fast_inline constexpr Vec2<T> & operator-=(const Vec2<auto> & b){
        x -= static_cast<T>(b.x);
        y -= static_cast<T>(b.y);
        return *this;
    }

    [[nodiscard]] __fast_inline constexpr Vec2<T> operator-() const{
        return Vec2<T> {-x, -y};
    }

    template<typename U>
    __fast_inline constexpr Vec2<T> & operator*=(const U n){
        x = static_cast<T>(x * n);
        y = static_cast<T>(y * n);
        return *this;
    }


    template<typename U>
    __fast_inline constexpr Vec2<T> & operator/=(const U n){
        if constexpr(std::is_integral_v<U>){
            x = x / n;
            y = y / n;
        }else{
            const T inv_n = static_cast<T>(1) / n;
            x = static_cast<T>(x * inv_n);
            y = static_cast<T>(y * inv_n);
        }
        return *this;
    }

    template<typename U>
    [[nodiscard]] __fast_inline constexpr Vec2<T> operator*(const U n) const{
        Vec2<T> ret = *this;
        return ret *= n;
    }

    template<typename U>
    [[nodiscard]] __fast_inline constexpr Vec2<T> operator/(const U n) const{
        Vec2<T> ret = *this;
        return ret /= n;
    }

    [[nodiscard]] constexpr bool is_zero() const {
        if constexpr(std::is_integral<T>::value){
            return x == 0 and y == 0;
        }else{
            return is_equal_approx(x, T(0)) and is_equal_approx(y, T(0));
        }
    }


    [[nodiscard]] __fast_inline static constexpr Vec2<T> from_ones(const T & len){
        return {len, len};
    }

    [[nodiscard]] __fast_inline constexpr Rect2<T> to_rect_with_another_corner(const Vec2<auto> & other) const {
        auto rect = Rect2<T>(other, other - *this);
        return rect.abs();
    }

    [[nodiscard]] __fast_inline constexpr Rect2<T> to_rect() const {
        return Rect2<T>(ZERO, *this);
    }

    [[nodiscard]] __fast_inline constexpr T area() const {
        return x * y;
    }



    [[nodiscard]] __fast_inline constexpr Rect2<T> overlap_as_rect(const Vec2<T> & other) const {
        return Rect2<T>({0,0}, {MIN(x, other.x), MIN(y, other.y)});
    }

    [[nodiscard]] __fast_inline constexpr Vec2<T> overlap_as_vec2(const Vec2<T> & other) const {
        return {MIN(x, other.x), MIN(y, other.y)};
    }

    [[nodiscard]] __fast_inline constexpr std::array<T, 2> to_array() const {
        return {x, y};
    }
};

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr Vec2<T> operator*(const T n, const Vec2<T> & vec){
    return vec * n;
}

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr auto lerp(const Vec2<T> & a, const Vec2<T> & b, const T t){
    return a + (b - a) * t;
}

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr auto distance(const Vec2<T> & from, const Vec2<T> & to){
    return (to - from).length();
}

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr auto normal(const Vec2<T> & from, const Vec2<T> & to){
    return (to - from).normalized();
}

__fast_inline OutputStream & operator<<(OutputStream & os, const Vec2<auto> & value){
    return os << os.brackets<'('>() << value.x << os.splitter() << value.y << os.brackets<')'>();
}

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec2u = Vec2<uint>;
using Vec2u8 = Vec2<uint8_t>;
using Vec2u16 = Vec2<uint16_t>;

}

namespace ymd{


template<arithmetic T>
constexpr Vec2<T> Vec2<T>::abs() const{
    return Vec2<T>(fabs(x), fabs(y));
}



template<arithmetic T>
constexpr Vec2<T> Vec2<T>::clamp(const T & _min, const T & _max) const {
    T min = static_cast<T>(_min);
    T max = static_cast<T>(_max);
    T l = this->length();
    if (l > max){
        return *this * max / l;
    }
    else if(l < min){
        return *this * min / l;
    }
    return *this;
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::dir_to(const Vec2<T> & b) const{
    return (b - *this).normalized();
}

template<arithmetic T>
constexpr T Vec2<T>::dist_to(const Vec2<T> & b) const{
    return (b - *this).length();
}

template<arithmetic T>
constexpr T Vec2<T>::dist_squared_to(const Vec2<T> & b) const{
    return (b - *this).length_squared();
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::reflect(const Vec2<T> & n) const {
    return 2 * n * this->dot(n) - *this;
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::bounce(const Vec2<T> & n) const {
    return -reflect(n);
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::lerp(const Vec2<T> & b, const T _t) const{
    static_assert(not std::is_integral_v<T>);
    return *this * (1-_t)+b * _t;
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::slerp(const Vec2<T> & b, const T _t) const{
    static_assert(not std::is_integral_v<T>);
    return lerp(b, sin(static_cast<T>(PI / 2) * _t));
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::posmod(const T & mod) const{
    return Vec2<T>( fmod(x, mod), fmod(y, mod));
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::posmodv(const Vec2<T> & b) const{
    return Vec2<T>(fmod(x, b.x), fmod(y, b.y));
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::project(const Vec2<T> & b) const{
    return (this->dot(b)) * b / b.length_squared();
}

template<arithmetic T>
constexpr T Vec2<T>::project(const T & rad) const{
    auto [s,c] = sincos(rad);
    return (this->x) * c + (this->y) * s;

}

template<arithmetic T>
constexpr bool Vec2<T>::is_equal_approx(const Vec2<T> & b) const{
    return ymd::is_equal_approx(x, b.x) && ymd::is_equal_approx(y, b.y);
}

template<arithmetic T>
constexpr bool Vec2<T>::has_point(const Vec2<auto> & _v) const{
    bool ret = true;
    Vec2<T> v = _v;

    if(x < 0) ret &= (x <= v.x && v.x <= 0);
    else ret &= (0 <= v.x && v.x <= x);
    if(y < 0) ret &= (y <= v.y && v.y <= 0);
    else ret &= (0 <= v.y && v.y <= y);

    return ret;
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::move_toward(const Vec2<T> & b, const T delta) const{
    if (!is_equal_approx(b)){
        Vec2<T> d = b - *this;
        return *this + d.clampmax(delta);
    }
    return *this;
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::slide(const Vec2<T> & n) const {
    return *this - n * this->dot(n);
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::sign() const{
    return Vec2<T>(sign(x), sign(y));
}

template<arithmetic T>
constexpr Vec2<T> Vec2<T>::snapped(const Vec2<T> &by) const{
    return Vec2<T>(snap(x, by.x), snap(y, by.y));
}

template<arithmetic T>
constexpr __fast_inline Vec2<T> Vec2<T>::normalized() const{
    static_assert(not std::is_integral_v<T>);
    return (*this) * isqrt(this->length_squared());
}

template<arithmetic T>
constexpr __fast_inline T Vec2<T>::dot(const Vec2<T> & with) const{
    return (x*with.x + y*with.y);
}

template<arithmetic T>
constexpr __fast_inline T Vec2<T>::cross(const Vec2<T> & with) const{
    return (x*with.y - y*with.x);
}


template<arithmetic T>
constexpr __fast_inline Vec2<T> Vec2<T>::improduct(const Vec2<T> & b) const{
    return Vec2<T>(x*b.x - y*b.y, x*b.y + y*b.x);
}

template<arithmetic T>
constexpr __fast_inline Vec2<T> Vec2<T>::rotated(const Angle<T> angle) const{
    static_assert(not std::is_integral_v<T>);
    return this->improduct(Vec2<T>::from_angle(angle));
}

#define VECTOR2_COMPARE_IM_OPERATOR(op) \
\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const Vec2<T>& lhs, const U& rhs) { \
    T absrhs = static_cast<T>(abs(rhs)); \
    return lhs.length_squared() op (absrhs * absrhs); \
} \
\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const U& lhs, const Vec2<T>& rhs) { \
    T abslhs = static_cast<T>(abs(lhs)); \
    return (abslhs * abslhs) op rhs.length_squared(); \
}\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const Vec2<T>& lhs, const Vec2<U>& rhs) { \
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);\
}\


VECTOR2_COMPARE_IM_OPERATOR(<)
VECTOR2_COMPARE_IM_OPERATOR(<=)
VECTOR2_COMPARE_IM_OPERATOR(>)
VECTOR2_COMPARE_IM_OPERATOR(>=)
VECTOR2_COMPARE_IM_OPERATOR(==)
VECTOR2_COMPARE_IM_OPERATOR(!=)

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vec2<T> operator +(const Vec2<T> &p_vector2, const Vec2<U> &d_vector2){
    Vec2<T> ret = p_vector2;
    ret += d_vector2;
    return ret;
}

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vec2<T> operator -(const Vec2<T> &p_vector2, const Vec2<U> &d_vector2){
    Vec2<T> ret = p_vector2;
    ret -= d_vector2;
    return ret;
}

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vec2<T> operator *(const T &lvalue, const Vec2<U> &p_vector2){
    Vec2<T> ret = p_vector2;
    ret *= lvalue;
    return ret;
}


template <arithmetic T,arithmetic U = T >
constexpr Vec2<T> operator/(const Vec2<T> &p_vector2, const Vec2<U> &d_vector2){
    Vec2<T> final = p_vector2;
    final /= d_vector2;
    return final;
}

template<arithmetic T>
Vec2() -> Vec2<T>;

}

namespace std{
    template<size_t I, typename T>
    static constexpr T get(const ymd::Vec2<T> &p_vector2){
        if constexpr(I == 0) return p_vector2.x;
        else if constexpr(I == 1) return p_vector2.y;
    }
}
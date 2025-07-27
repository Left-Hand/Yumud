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

namespace ymd{

template<arithmetic T>
struct Rect2;

template<arithmetic T>
struct Vector2{
    static constexpr Vector2<T> ZERO = Vector2<T>(0, 0);
    static constexpr Vector2<T> ONE = Vector2<T>(1, 1);
    static constexpr Vector2<T> INF = Vector2<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());

    static constexpr Vector2<T> LEFT = Vector2<T>(-1, 0);
    static constexpr Vector2<T> RIGHT = Vector2<T>(1, 0);

    static constexpr Vector2<T> X_AXIS = Vector2<T>(1, 0);
    static constexpr Vector2<T> Y_AXIS = Vector2<T>(0, 1);

    static constexpr Vector2<T> UP = Vector2<T>(0, 1);
    static constexpr Vector2<T> DOWN = Vector2<T>(0, -1);

    static constexpr Vector2<T> LEFT_UP = Vector2<T>(-1, 1);
    static constexpr Vector2<T> RIGHT_UP = Vector2<T>(1, 1);
    static constexpr Vector2<T> LEFT_DOWN = Vector2<T>(-1, -1);
    static constexpr Vector2<T> RIGHT_DOWN = Vector2<T>(1, -1);


    T x;
    T y;
    
    
    [[nodiscard]] constexpr Vector2():
        x(T(0)),
        y(T(0)){;}

    [[nodiscard]] constexpr Vector2(const T _x, const T _y): 
        x(T(_x)), y(T(_y)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Vector2(const std::tuple<U, U> & v) : 
        x(std::get<0>(v)), y(std::get<1>(v)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Vector2(const Vector2<U> & _v) : 
        x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    [[nodiscard]] __fast_inline static constexpr Vector2<T> from_idenity_rotation(const T & rad){
        const auto [s,c] = sincos(rad);
        return {c, s};
    }
    
    [[nodiscard]] T & operator [](const size_t index) { return *(&this->x + index);}

    [[nodiscard]] const T & operator [](const size_t index) const {return *(&this->x + index);}


    [[nodiscard]] static constexpr bool sort_by_x(const Vector2 & a, const Vector2 & b){
        return a.x < b.x;
    };

    [[nodiscard]] static constexpr bool sort_by_y(const Vector2 & a, const Vector2 & b){
        return a.y < b.y;
    };

    [[nodiscard]] static constexpr bool sort_by_length(const Vector2 & a, const Vector2 & b){
        return a.length_squared() < b.length_squared();
    };

    [[nodiscard]] static constexpr bool sort_by_angle(const Vector2 & a, const Vector2 & b){
        return a.cross(b) > 0;
    };

    [[nodiscard]] constexpr Vector2<T> normalized() const;
    [[nodiscard]] constexpr T cross(const Vector2<T> & other) const;

    [[nodiscard]] __fast_inline constexpr bool is_clockwise_to(const Vector2<T> & other) const{
        return (x*other.y > y*other.x);
    }

    [[nodiscard]] __fast_inline constexpr bool is_count_clockwise_to(const Vector2<T> & other) const{
        return (x*other.y < y*other.x);
    }

    [[nodiscard]] constexpr T dot(const Vector2<T> & other) const;
    [[nodiscard]] constexpr Vector2<T> improduct(const Vector2<T> & b) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> rotated(const T r)const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> abs() const;



    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector2<T> increase_x(const U & v){
        return {x + v, y};
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector2<T> increase_y(const U & v){
        return {x, y + v};
    }

    [[nodiscard]] constexpr T angle() const {
            return atan2(y, x);}
	[[nodiscard]] constexpr T angle_between(const Vector2<T> & b) const {
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
        
        return diff;
    }

    [[nodiscard]] constexpr T aspect() const {return (!!y) ? x/y : T(0);}
    [[nodiscard]] constexpr Vector2<T> bounce(const Vector2<T> & n) const;
    [[nodiscard]] constexpr Vector2<T> ceil() const;

    template<arithmetic U>
    [[nodiscard]] constexpr Vector2<T> clampmin(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l < length ? *this * length / l : *this);
    }

    template<arithmetic U>
    [[nodiscard]] constexpr Vector2<T> clampmax(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l >= length ? (this->normalized() * length) : *this);
    }

    [[nodiscard]] constexpr Vector2<T> clamp(const arithmetic auto & _min, const arithmetic auto & _max) const;

    [[nodiscard]] constexpr Vector2<T> dir_to(const Vector2<T> & b) const;
    [[nodiscard]] constexpr T dist_to(const Vector2<T> & b) const;
    [[nodiscard]] constexpr T dist_squared_to(const Vector2<T> & b) const;
    [[nodiscard]] constexpr Vector2<T> floor() const;
    [[nodiscard]] constexpr bool is_equal_approx(const Vector2<T> & v) const;
    [[nodiscard]] constexpr T manhattan_distance()const{
        return ABS(x) + ABS(y);
    }


    [[nodiscard]] constexpr bool has_point(const Vector2<auto> & _v)const;
    [[nodiscard]] constexpr bool is_normalized() const {return (fabs(x*x + y*y + T(-1)) <= T(CMP_EPSILON));}
    [[nodiscard]] constexpr T length() const {
        return mag(x,y);
    }

    [[nodiscard]] constexpr T inv_length() const {
        return imag(x,y);
    }
    
    [[nodiscard]] constexpr T length_squared() const {return (x*x + y*y);}
    
    [[nodiscard]] __fast_inline constexpr Vector2<T> lerp(const Vector2<T> & b, const T t) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> move_toward(const Vector2<T> & to, const T delta) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> midpoint_with(const Vector2<T> other) const 
        {return {(this->x + other.x) / 2, (this->y + other.y) / 2};}

    [[nodiscard]] __fast_inline constexpr Vector2<T> posmod(const arithmetic auto & mod) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> posmodv(const Vector2<T> & modv) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> project(const Vector2<T> & b) const;
    [[nodiscard]] __fast_inline constexpr T project(const T & rad) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> reflect(const Vector2<T> & n) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> round() const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> sign() const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> slerp(const Vector2<T> & b, const T t) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> slide(const Vector2<T>  & n) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> snapped(const Vector2<T> & by) const;
    [[nodiscard]] __fast_inline constexpr Vector2<T> cw() const {return Vector2<T>(-y, x);}
    [[nodiscard]] __fast_inline constexpr Vector2<T> ccw() const {return Vector2<T>(y, -x);}

    [[nodiscard]] __fast_inline constexpr Vector2<T> flip_y() const {return {x,-y};}
    [[nodiscard]] __fast_inline constexpr Vector2<T> flip_x() const {return {-x,y};}

    [[nodiscard]] __fast_inline constexpr Vector2<T> swap_xy() const {return {y,x};}

    __fast_inline constexpr Vector2<T> & operator=(const Vector2<auto> & b){
        x = static_cast<T>(b.x);
        y = static_cast<T>(b.y);
        return *this;
    }


    __fast_inline constexpr Vector2<T> & operator+=(const Vector2<auto> & b){
        x += static_cast<T>(b.x);
        y += static_cast<T>(b.y);
        return *this;
    }

    __fast_inline constexpr Vector2<T> & operator-=(const Vector2<auto> & b){
        x -= static_cast<T>(b.x);
        y -= static_cast<T>(b.y);
        return *this;
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> operator-() const{
        Vector2<T> ret;
        ret.x = -x;
        ret.y = -y;
        return ret;
    }

    __fast_inline constexpr Vector2<T> & operator*=(const arithmetic auto & n){
        // using CommonType = typename std::common_type<T, decltype(n)>::type;/
        // using CommonType = T;
        x = static_cast<T>(x * n);
        y = static_cast<T>(y * n);
        return *this;
    }

    __fast_inline constexpr Vector2<T> & operator/=(const arithmetic auto & n){
        // using CommonType = typename std::common_type<T, decltype(n)>::type;
        if constexpr(std::is_integral_v<T>){
            x = x / n;
            y = y / n;
        }else{
            const T inv_n = static_cast<T>(1) / n;
            x = static_cast<T>(x * inv_n);
            y = static_cast<T>(y * inv_n);
        }
        return *this;
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> operator*(const arithmetic auto & n) const{
        Vector2<T> ret = *this;
        return ret *= n;
    }
    [[nodiscard]] __fast_inline constexpr Vector2<T> operator/(const arithmetic auto & n) const{
        Vector2<T> ret = *this;
        return ret /= n;
    }

    [[nodiscard]] constexpr bool is_zero() const {
        if constexpr(std::is_integral<T>::value){
            return x == 0 and y == 0;
        }else{
            return is_equal_approx(x, T(0)) and is_equal_approx(y, T(0));
        }
    }


    [[nodiscard]] __fast_inline static constexpr Vector2<T> from_ones(const T & len){
        return {len, len};
    }

    [[nodiscard]] __fast_inline constexpr Rect2<T> to_rect_with_another_corner(const Vector2<auto> & other) const {
        auto rect = Rect2<T>(other, other - *this);
        return rect.abs();
    }

    [[nodiscard]] __fast_inline constexpr Rect2<T> to_rect() const {
        return Rect2<T>(ZERO, *this);
    }

    [[nodiscard]] __fast_inline constexpr T area() const {
        return x * y;
    }



    [[nodiscard]] __fast_inline constexpr Rect2<T> overlap_as_rect(const Vector2<T> & other) const {
        return Rect2<T>({0,0}, {MIN(x, other.x), MIN(y, other.y)});
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> overlap_as_vec2(const Vector2<T> & other) const {
        return {MIN(x, other.x), MIN(y, other.y)};
    }
};

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr Vector2<T> operator*(const arithmetic auto & n, const Vector2<T> & vec){
    return vec * n;
}

[[nodiscard]] __fast_inline constexpr auto lerp(const Vector2<arithmetic auto> & a, const Vector2<arithmetic auto> & b, const arithmetic auto t){
    return a + (b - a) * t;
}

[[nodiscard]] __fast_inline constexpr auto distance(const Vector2<arithmetic auto> & from, const Vector2<arithmetic auto> & to){
    return (to - from).length();
}

[[nodiscard]] __fast_inline constexpr auto normal(const Vector2<arithmetic auto> & from, const Vector2<arithmetic auto> & to){
    return (to - from).normalized();
}

__fast_inline OutputStream & operator<<(OutputStream & os, const Vector2<auto> & value){
    return os << os.brackets<'('>() << value.x << os.splitter() << value.y << os.brackets<')'>();
}

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<uint>;
using Vector2u8 = Vector2<uint8_t>;

}

#include "vector2.tpp"

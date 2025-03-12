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


#include "sys/stream/ostream.hpp"
#include "sys/math/real.hpp"

namespace ymd{

template<arithmetic T>
struct Rect2_t;

template<arithmetic T>
struct Vector2_t{
public:
    T x;
    T y;
    
    constexpr Vector2_t(){;}

    constexpr Vector2_t(const auto & _x, const auto & _y): x(T(_x)), y(T(_y)){;}

    template<arithmetic U = T>
    constexpr Vector2_t(const std::tuple<U, U> & v) : x(std::get<0>(v)), y(std::get<1>(v)){;}

    template<arithmetic U = T>
    constexpr Vector2_t(const Vector2_t<U> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    T & operator [](const size_t index) { return *(&this->x + index);}

    const T & operator [](const size_t index) const {return *(&this->x + index);}

    scexpr Vector2_t<T> ZERO = Vector2_t<T>(0, 0);
    scexpr Vector2_t<T> ONE = Vector2_t<T>(1, 1);
    scexpr Vector2_t<T> INF = Vector2_t<T>(INFINITY, INFINITY);

    scexpr Vector2_t<T> LEFT = Vector2_t<T>(-1, 0);
    scexpr Vector2_t<T> RIGHT = Vector2_t<T>(1, 0);
    scexpr Vector2_t<T> UP = Vector2_t<T>(0, 1);
    scexpr Vector2_t<T> DOWN = Vector2_t<T>(0, -1);

    scexpr Vector2_t<T> LEFT_UP = Vector2_t<T>(-1, 1);
    scexpr Vector2_t<T> RIGHT_UP = Vector2_t<T>(1, 1);
    scexpr Vector2_t<T> LEFT_DOWN = Vector2_t<T>(-1, -1);
    scexpr Vector2_t<T> RIGHT_DOWN = Vector2_t<T>(1, -1);

    constexpr Vector2_t<T> normalize(){*this /= this->length();}
    constexpr Vector2_t<T> normalized() const;
    constexpr T cross(const Vector2_t<T> & with) const;
    constexpr T dot(const Vector2_t<T> & with) const;
    constexpr Vector2_t<T> improduct(const Vector2_t<T> & b) const;
    constexpr Vector2_t<T> rotated(const T r)const;
    constexpr Vector2_t<T> abs() const;


    static bool sort_by_x(const Vector2_t & a, const Vector2_t & b){
        return a.x < b.x;
    };

    static bool sort_by_y(const Vector2_t & a, const Vector2_t & b){
        return a.y < b.y;
    };

    static bool sort_by_length(const Vector2_t & a, const Vector2_t & b){
        return a.length_squared() < b.length_squared();
    };

    static bool sort_by_angle(const Vector2_t & a, const Vector2_t & b){
        return a.cross(b) > 0;
    };


    template<arithmetic U>
    __fast_inline constexpr Vector2_t<T> increase_x(const U & v){
        return {x + v, y};
    }

    template<arithmetic U>
    __fast_inline constexpr Vector2_t<T> increase_y(const U & v){
        return {x, y + v};
    }

    // template<arithmetic U>
    // constexpr T cos(const Vector2_t<U> & b) const{
    //     return this->dot(b) / this->length() / b.length();
    // }

    // template<arithmetic U>
    // constexpr T sin(const Vector2_t<U> & b) const{
    //     return this->cross(b) / this->length() / b.length();
    // }

    constexpr T angle() const {return atan2(y, x);}
	constexpr T angle_to(const Vector2_t<T> &p_vector2) const {return atan2(cross(p_vector2), dot(p_vector2));}
	constexpr T angle_to_point(const Vector2_t<T> & p_vector2) const {return atan2(y - p_vector2.y, x - p_vector2.x);}
    constexpr T aspect() const {return (!!y) ? x/y : T(0);}
    constexpr Vector2_t<T> bounce(const Vector2_t<T> & n) const;
    constexpr Vector2_t<T> ceil() const;

    template<arithmetic U>
    constexpr Vector2_t<T> clampmin(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l < length ? *this * length / l : *this);
    }

    template<arithmetic U>
    constexpr Vector2_t<T> clampmax(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l > length ? *this * length / l : *this);
    }

    constexpr Vector2_t<T> clamp(const arithmetic auto & _min, const arithmetic auto & _max) const;

    constexpr Vector2_t<T> dir_to(const Vector2_t<T> & b) const;
    constexpr T dist_to(const Vector2_t<T> & b) const;
    constexpr T dist_squared_to(const Vector2_t<T> & b) const;
    constexpr Vector2_t<T> floor() const;
    constexpr bool is_equal_approx(const Vector2_t<T> & v) const;
    __inline constexpr T manhattan_distance()const{
        return ABS(x) + ABS(y);
    }


    constexpr bool has_point(const Vector2_t<auto> & _v)const;
    constexpr bool is_normalized() const {return (fabs(x*x + y*y + T(-1)) <= T(CMP_EPSILON));}
    constexpr T length() const {
        return imag(x,y);
    }
    
    constexpr T length_squared() const {return (x*x + y*y);}
    
    __fast_inline constexpr Vector2_t<T> lerp(const Vector2_t<T> & b, const arithmetic auto & t) const;
    __fast_inline constexpr Vector2_t<T> move_toward(const Vector2_t<T> & to, const arithmetic auto & delta) const;
    __fast_inline constexpr Vector2_t<T> center(const Vector2_t<T> other) const 
        {return {(this->x + other.x) / 2, (this->y + other.y) / 2};}

    __fast_inline constexpr Vector2_t<T> posmod(const arithmetic auto & mod) const;
    __fast_inline constexpr Vector2_t<T> posmodv(const Vector2_t<T> & modv) const;
    __fast_inline constexpr Vector2_t<T> project(const Vector2_t<T> & b) const;
    __fast_inline constexpr T project(const T & rad) const;
    __fast_inline constexpr Vector2_t<T> reflect(const Vector2_t<T> & n) const;
    __fast_inline constexpr Vector2_t<T> round() const;
    __fast_inline constexpr Vector2_t<T> sign() const;
    __fast_inline constexpr Vector2_t<T> slerp(const Vector2_t<T> & b, const arithmetic auto & t) const;
    __fast_inline constexpr Vector2_t<T> slide(const Vector2_t<T>  & n) const;
    __fast_inline constexpr Vector2_t<T> snapped(const Vector2_t<T> & by) const;
    __fast_inline constexpr Vector2_t<T> cw() const {return Vector2_t<T>(-y, x);}
    __fast_inline constexpr Vector2_t<T> ccw() const {return Vector2_t<T>(y, -x);}

    __fast_inline constexpr Vector2_t<T> flipy() const {return {x,-y};}
    __fast_inline constexpr Vector2_t<T> flipx() const {return {-x,y};}

    __fast_inline constexpr Vector2_t<T> swapxy() const {return {y,x};}

    __fast_inline constexpr Vector2_t<T> & operator=(const Vector2_t<auto> & b){
        x = static_cast<T>(b.x);
        y = static_cast<T>(b.y);
        return *this;
    }


    __fast_inline constexpr Vector2_t<T> & operator+=(const Vector2_t<auto> & b){
        x += static_cast<T>(b.x);
        y += static_cast<T>(b.y);
        return *this;
    }

    __fast_inline constexpr Vector2_t<T> & operator-=(const Vector2_t<auto> & b){
        x -= static_cast<T>(b.x);
        y -= static_cast<T>(b.y);
        return *this;
    }

    __fast_inline constexpr Vector2_t<T> operator-() const{
        Vector2_t<T> ret;
        ret.x = -x;
        ret.y = -y;
        return ret;
    }

    __fast_inline constexpr Vector2_t<T> & operator*=(const arithmetic auto & n){
        // using CommonType = typename std::common_type<T, decltype(n)>::type;/
        // using CommonType = T;
        x = static_cast<T>(x * n);
        y = static_cast<T>(y * n);
        return *this;
    }

    __fast_inline constexpr Vector2_t<T> & operator/=(const arithmetic auto & n){
        // using CommonType = typename std::common_type<T, decltype(n)>::type;
        const T inv_n = static_cast<T>(1) / n;
        x = static_cast<T>(x * inv_n);
        y = static_cast<T>(y * inv_n);
        return *this;
    }

    __fast_inline constexpr Vector2_t<T> operator*(const arithmetic auto & n) const{
        Vector2_t<T> ret = *this;
        return ret *= n;
    }
    __fast_inline constexpr Vector2_t<T> operator/(const arithmetic auto & n) const{
        Vector2_t<T> ret = *this;
        return ret /= n;
    }

    constexpr explicit operator bool() const {
        if constexpr(std::is_integral<T>::value){
            return x != 0 || y != 0;
        }else{
            return !is_equal_approx(x, T(0)) || !is_equal_approx(y, T(0));
        }
    }

    __fast_inline static constexpr Vector2_t<T> from_angle(const T & len, const T & rad){
        return {len * cos(rad), len * sin(rad)};
    }

    __fast_inline static constexpr Vector2_t<T> ones(const T & len){
        return {len, len};
    }

    __fast_inline constexpr Rect2_t<T> form_rect(const Vector2_t<auto> & other) const {
        auto rect = Rect2_t<T>(other, other - *this);
        return rect.abs();
    }

    __fast_inline constexpr T area() const {
        return x * y;
    }
};

template<arithmetic T>
__fast_inline constexpr Vector2_t<T> operator*(const arithmetic auto & n, const Vector2_t<T> & vec){
    return vec * n;
}


using Vector2i = Vector2_t<int>;
using Vector2 = Vector2_t<real_t>;

__fast_inline OutputStream & operator<<(OutputStream & os, const Vector2_t<auto> & value){
    return os << os.brackets<'('>() << value.x << os.splitter() << value.y << os.brackets<')'>();
}


__fast_inline constexpr auto lerp(const Vector2_t<arithmetic auto> & a, const Vector2_t<arithmetic auto> & b, const arithmetic auto t){
    return a + (b - a) * t;
}

__fast_inline constexpr auto distance(const Vector2_t<arithmetic auto> & from, const Vector2_t<arithmetic auto> & to){
    return (to - from).length();
}

__fast_inline constexpr auto normal(const Vector2_t<arithmetic auto> & from, const Vector2_t<arithmetic auto> & to){
    return (to - from).normalized();
}

}




#include "vector2.tpp"

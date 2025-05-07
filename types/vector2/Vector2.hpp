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
struct Rect2_t;

template<arithmetic T>
struct Vector2_t{
public:
    T x;
    T y;
    
    
    [[nodiscard]] constexpr Vector2_t(){;}

    [[nodiscard]] constexpr Vector2_t(const T _x, const T _y): x(T(_x)), y(T(_y)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Vector2_t(const std::tuple<U, U> & v) : x(std::get<0>(v)), y(std::get<1>(v)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Vector2_t(const Vector2_t<U> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    [[nodiscard]] T & operator [](const size_t index) { return *(&this->x + index);}

    [[nodiscard]] const T & operator [](const size_t index) const {return *(&this->x + index);}

    scexpr Vector2_t<T> ZERO = Vector2_t<T>(0, 0);
    scexpr Vector2_t<T> ONE = Vector2_t<T>(1, 1);
    scexpr Vector2_t<T> INF = Vector2_t<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());

    scexpr Vector2_t<T> LEFT = Vector2_t<T>(-1, 0);
    scexpr Vector2_t<T> RIGHT = Vector2_t<T>(1, 0);
    scexpr Vector2_t<T> UP = Vector2_t<T>(0, 1);
    scexpr Vector2_t<T> DOWN = Vector2_t<T>(0, -1);

    scexpr Vector2_t<T> LEFT_UP = Vector2_t<T>(-1, 1);
    scexpr Vector2_t<T> RIGHT_UP = Vector2_t<T>(1, 1);
    scexpr Vector2_t<T> LEFT_DOWN = Vector2_t<T>(-1, -1);
    scexpr Vector2_t<T> RIGHT_DOWN = Vector2_t<T>(1, -1);

    [[nodiscard]] constexpr Vector2_t<T> normalize(){*this /= this->length();}
    [[nodiscard]] constexpr Vector2_t<T> normalized() const;
    [[nodiscard]] constexpr T cross(const Vector2_t<T> & with) const;
    [[nodiscard]] constexpr T dot(const Vector2_t<T> & with) const;
    [[nodiscard]] constexpr Vector2_t<T> improduct(const Vector2_t<T> & b) const;
    [[nodiscard]] constexpr Vector2_t<T> rotated(const T r)const;
    [[nodiscard]] constexpr Vector2_t<T> abs() const;


    [[nodiscard]] static bool sort_by_x(const Vector2_t & a, const Vector2_t & b){
        return a.x < b.x;
    };

    [[nodiscard]] static bool sort_by_y(const Vector2_t & a, const Vector2_t & b){
        return a.y < b.y;
    };

    [[nodiscard]] static bool sort_by_length(const Vector2_t & a, const Vector2_t & b){
        return a.length_squared() < b.length_squared();
    };

    [[nodiscard]] static bool sort_by_angle(const Vector2_t & a, const Vector2_t & b){
        return a.cross(b) > 0;
    };


    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> increase_x(const U & v){
        return {x + v, y};
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> increase_y(const U & v){
        return {x, y + v};
    }

    [[nodiscard]] constexpr T angle() const {return atan2(y, x);}
	[[nodiscard]] constexpr T angle_to(const Vector2_t<T> &p_vector2) const {return atan2(cross(p_vector2), dot(p_vector2));}
	[[nodiscard]] constexpr T angle_to_point(const Vector2_t<T> & p_vector2) const {return atan2(y - p_vector2.y, x - p_vector2.x);}
    [[nodiscard]] constexpr T aspect() const {return (!!y) ? x/y : T(0);}
    [[nodiscard]] constexpr Vector2_t<T> bounce(const Vector2_t<T> & n) const;
    [[nodiscard]] constexpr Vector2_t<T> ceil() const;

    template<arithmetic U>
    [[nodiscard]] constexpr Vector2_t<T> clampmin(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l < length ? *this * length / l : *this);
    }

    template<arithmetic U>
    [[nodiscard]] constexpr Vector2_t<T> clampmax(const U & _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l >= length ? (this->normalized() * length) : *this);
    }

    [[nodiscard]] constexpr Vector2_t<T> clamp(const arithmetic auto & _min, const arithmetic auto & _max) const;

    [[nodiscard]] constexpr Vector2_t<T> dir_to(const Vector2_t<T> & b) const;
    [[nodiscard]] constexpr T dist_to(const Vector2_t<T> & b) const;
    [[nodiscard]] constexpr T dist_squared_to(const Vector2_t<T> & b) const;
    [[nodiscard]] constexpr Vector2_t<T> floor() const;
    [[nodiscard]] constexpr bool is_equal_approx(const Vector2_t<T> & v) const;
    [[nodiscard]] constexpr T manhattan_distance()const{
        return ABS(x) + ABS(y);
    }


    [[nodiscard]] constexpr bool has_point(const Vector2_t<auto> & _v)const;
    [[nodiscard]] constexpr bool is_normalized() const {return (fabs(x*x + y*y + T(-1)) <= T(CMP_EPSILON));}
    [[nodiscard]] constexpr T length() const {
        return mag(x,y);
    }

    [[nodiscard]] constexpr T inv_length() const {
        return imag(x,y);
    }
    
    [[nodiscard]] constexpr T length_squared() const {return (x*x + y*y);}
    
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> lerp(const Vector2_t<T> & b, const arithmetic auto & t) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> move_toward(const Vector2_t<T> & to, const arithmetic auto & delta) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> center(const Vector2_t<T> other) const 
        {return {(this->x + other.x) / 2, (this->y + other.y) / 2};}

    [[nodiscard]] __fast_inline constexpr Vector2_t<T> posmod(const arithmetic auto & mod) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> posmodv(const Vector2_t<T> & modv) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> project(const Vector2_t<T> & b) const;
    [[nodiscard]] __fast_inline constexpr T project(const T & rad) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> reflect(const Vector2_t<T> & n) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> round() const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> sign() const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> slerp(const Vector2_t<T> & b, const arithmetic auto & t) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> slide(const Vector2_t<T>  & n) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> snapped(const Vector2_t<T> & by) const;
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> cw() const {return Vector2_t<T>(-y, x);}
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> ccw() const {return Vector2_t<T>(y, -x);}

    [[nodiscard]] __fast_inline constexpr Vector2_t<T> flipy() const {return {x,-y};}
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> flipx() const {return {-x,y};}

    [[nodiscard]] __fast_inline constexpr Vector2_t<T> swapxy() const {return {y,x};}

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

    [[nodiscard]] __fast_inline constexpr Vector2_t<T> operator-() const{
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

    [[nodiscard]] __fast_inline constexpr Vector2_t<T> operator*(const arithmetic auto & n) const{
        Vector2_t<T> ret = *this;
        return ret *= n;
    }
    [[nodiscard]] __fast_inline constexpr Vector2_t<T> operator/(const arithmetic auto & n) const{
        Vector2_t<T> ret = *this;
        return ret /= n;
    }

    [[nodiscard]] constexpr explicit operator bool() const {
        if constexpr(std::is_integral<T>::value){
            return x != 0 || y != 0;
        }else{
            return !is_equal_approx(x, T(0)) || !is_equal_approx(y, T(0));
        }
    }

    [[nodiscard]] __fast_inline static constexpr Vector2_t<T> from_angle(const T & len, const T & rad){
        return {len * cos(rad), len * sin(rad)};
    }

    [[nodiscard]] __fast_inline static constexpr Vector2_t<T> ones(const T & len){
        return {len, len};
    }

    [[nodiscard]] __fast_inline constexpr Rect2_t<T> form_rect(const Vector2_t<auto> & other) const {
        auto rect = Rect2_t<T>(other, other - *this);
        return rect.abs();
    }

    [[nodiscard]] __fast_inline constexpr T area() const {
        return x * y;
    }
};

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr Vector2_t<T> operator*(const arithmetic auto & n, const Vector2_t<T> & vec){
    return vec * n;
}

[[nodiscard]] __fast_inline constexpr auto lerp(const Vector2_t<arithmetic auto> & a, const Vector2_t<arithmetic auto> & b, const arithmetic auto t){
    return a + (b - a) * t;
}

[[nodiscard]] __fast_inline constexpr auto distance(const Vector2_t<arithmetic auto> & from, const Vector2_t<arithmetic auto> & to){
    return (to - from).length();
}

[[nodiscard]] __fast_inline constexpr auto normal(const Vector2_t<arithmetic auto> & from, const Vector2_t<arithmetic auto> & to){
    return (to - from).normalized();
}

__fast_inline OutputStream & operator<<(OutputStream & os, const Vector2_t<auto> & value){
    return os << os.brackets<'('>() << value.x << os.splitter() << value.y << os.brackets<')'>();
}



using Vector2 = Vector2_t<real_t>;
using Vector2i = Vector2_t<int>;
using Vector2u = Vector2_t<uint>;

}


// namespace std{
//     template<typename T>
//     struct tuple_size<ymd::Vector2_t<T>> {
//         constexpr static size_t value = 2;
//     };

//     template<size_t N, typename T>
//     struct tuple_element<N, ymd::Vector2_t<T>> {
//         using type = T;
//     };

//     template<size_t N, typename T>
//     auto get(const ymd::Vector2_t<T> & v){
//         static_assert(N < 2);
//         if constexpr (N == 0) return v.x;
//         else return v.y;
//     }
// }

#include "vector2.tpp"

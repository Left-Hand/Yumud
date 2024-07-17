#ifndef __VECTOR2_HPP_

#define __VECTOR2_HPP_

#include "../sys/core/platform.h"
#include "../types/string/String.hpp"
#include "../types/real.hpp"


#include <type_traits>
#include <tuple>

template<arithmetic T>
struct Rect2_t;

template<arithmetic T>
struct Vector2_t{
public:

    T x = T(0);
    T y = T(0);

    __fast_inline constexpr Vector2_t(){;}

    template<arithmetic U, arithmetic V>
    __fast_inline constexpr Vector2_t(const U _x, const V _y): x(_x), y(_y){;}

    template<arithmetic U, arithmetic V>
    __fast_inline constexpr Vector2_t(const std::tuple<U, V> & v) : x(std::get<0>(v)), y(std::get<1>(v)){;}

    template<arithmetic U>
    __fast_inline constexpr Vector2_t(const Vector2_t<U> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    static constexpr Vector2_t<T> ZERO = Vector2_t<T>(0, 0);
    static constexpr Vector2_t<T> ONE = Vector2_t<T>(1, 1);
    static constexpr Vector2_t<T> LEFT = Vector2_t<T>(-1, 0);
    static constexpr Vector2_t<T> RIGHT = Vector2_t<T>(1, 0);
    static constexpr Vector2_t<T> UP = Vector2_t<T>(0, 1);
    static constexpr Vector2_t<T> DOWN = Vector2_t<T>(0, -1);
    static constexpr Vector2_t<T> LEFT_UP = Vector2_t<T>(-1, 1);
    static constexpr Vector2_t<T> RIGHT_UP = Vector2_t<T>(1, 1);
    static constexpr Vector2_t<T> LEFT_DOWN = Vector2_t<T>(-1, -1);
    static constexpr Vector2_t<T> RIGHT_DOWN = Vector2_t<T>(1, -1);

    __fast_inline_constexpr Vector2_t<T> normalize(){*this /= this->length();}
    __fast_inline_constexpr Vector2_t<T> normalized() const;
    __fast_inline_constexpr T cross(const Vector2_t<T> & with) const;
    __fast_inline_constexpr T dot(const Vector2_t<T> & with) const;
    __fast_inline_constexpr Vector2_t<T> improduct(const Vector2_t<T> & b) const;
    __fast_inline_constexpr Vector2_t<T> rotated(const T r)const;
    __fast_inline_constexpr Vector2_t<T> abs() const;
    __fast_inline_constexpr T cos(const Vector2_t<auto> & b) const;
    __fast_inline_constexpr T sin(const Vector2_t<auto> & b) const;
    __fast_inline T angle() const {return atan2(y, x);}
    constexpr T angle_to(const Vector2_t<T> & to) const;
    constexpr T angle_to_point(const Vector2_t<T> & to) const;
    constexpr T aspect() const {return (!!y) ? x/y : T(0);}
    constexpr Vector2_t<T> bounce(const Vector2_t<T> &n) const;
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

    // template<typename U>
    constexpr Vector2_t<T> clamp(const auto & _min, const auto & _max) const;
    constexpr Vector2_t<T> dir_to(const Vector2_t<T> & b) const;
    constexpr T dist_to(const Vector2_t<T> & b) const;
    constexpr T dist_squared_to(const Vector2_t<T> & b) const;
    constexpr Vector2_t<T> floor() const;
    constexpr bool is_equal_approx(const Vector2_t<T> & v) const;


    constexpr bool has_point(const Vector2_t<auto> & _v)const;
    constexpr bool is_normalized() const {return (fabs(x*x + y*y + T(-1)) <= T(CMP_EPSILON));}
    constexpr __fast_inline T length() const {
        auto c = length_squared();
        return c ? sqrt(c) : 0;}
    constexpr __fast_inline T length_squared() const {return (x*x + y*y);}
    constexpr Vector2_t<T> lerp(const Vector2_t<T> & b, const T & t) const;
    constexpr Vector2_t<T> move_toward(const Vector2_t<T> & to, const T & delta) const;

    constexpr Vector2_t<T> posmod(const T & mod) const;
    constexpr Vector2_t<T> posmodv(const Vector2_t<T> & modv) const;
    constexpr Vector2_t<T> project(const Vector2_t<T> & b) const;
    constexpr Vector2_t<T> reflect(const Vector2_t<T> & n) const;
    constexpr Vector2_t<T> round() const;
    constexpr Vector2_t<T> sign() const;
    constexpr Vector2_t<T> slerp(const Vector2_t<T> & b, const T & t) const;
    constexpr Vector2_t<T> slide(const Vector2_t<T>  & n) const;
    constexpr Vector2_t<T> snapped(const Vector2_t<T> & by) const;


    constexpr __fast_inline Vector2_t<T> & operator=(const Vector2_t<auto> & b){
        x = static_cast<T>(b.x);
        y = static_cast<T>(b.y);
        return *this;
    }


    constexpr __fast_inline Vector2_t<T> & operator+=(const Vector2_t<auto> & b){
        x += static_cast<T>(b.x);
        y += static_cast<T>(b.y);
        return *this;
    }

    constexpr __fast_inline Vector2_t<T> & operator-=(const Vector2_t<auto> & b){
        x -= static_cast<T>(b.x);
        y -= static_cast<T>(b.y);
        return *this;
    }

    constexpr __fast_inline Vector2_t<T> operator-() const{
        Vector2_t<T> ret;
        ret.x = -x;
        ret.y = -y;
        return ret;
    }

    constexpr __fast_inline Vector2_t<T> & operator*=(const auto & n){
        using CommonType = typename std::common_type<T, decltype(n)>::type;
        x = static_cast<T>(static_cast<CommonType>(x) * n);
        y = static_cast<T>(static_cast<CommonType>(y) * n);
        return *this;
    }

    constexpr __fast_inline Vector2_t<T> & operator/=(const auto & n){
        using CommonType = typename std::common_type<T, decltype(n)>::type;
        x = static_cast<T>(static_cast<CommonType>(x) / n);
        y = static_cast<T>(static_cast<CommonType>(y) / n);
        return *this;
    }

    __fast_inline Vector2_t<T> operator*(const auto & n) const{
        Vector2_t<T> ret = *this;
        return ret *= n;
    }
    __fast_inline Vector2_t<T> operator/(const auto & n) const{
        Vector2_t<T> ret = *this;
        return ret /= n;
    }

    __fast_inline_constexpr explicit operator bool() const {
        if constexpr(std::is_integral<T>::value){
            return x != 0 || y != 0;
        }else{
            return !::is_equal_approx(x, T(0)) || !::is_equal_approx(y, T(0));
        }
    }
    constexpr  __no_inline explicit operator String() const{
        return toString();
    }

    __no_inline String toString(unsigned char decimalPlaces = 2) const {
        if constexpr(std::is_integral<T>::value){
            return ('(' + String(x) + ',' + String(y) + ')');
        }else{
            return ('(' + ::toString(x, decimalPlaces) + ',' + ::toString(y, decimalPlaces) + ')');
        }
    }



    Rect2_t<T> form_rect() const {
        return Rect2_t<T>(Vector2_t<T>(), *this);
    }

    Rect2_t<T> form_rect(const Vector2_t<auto> & other) const {
        auto rect = Rect2_t<T>(other, other - *this);
        return rect.abs();
    }
};

#include "vector2_t.tpp"

using Vector2f = Vector2_t<float>;
using Vector2i = Vector2_t<int>;
using Vector2 = Vector2_t<real_t>;

__fast_inline OutputStream & operator<<(OutputStream & os, const Vector2_t<auto> & value){
    return os << '(' << value.x << ',' << value.y << ')';
}
#endif

#ifndef __VECTOR2_HPP_

#define __VECTOR2_HPP_

#include "src/platform.h"
#include "../string/String.hpp"
#include "../real.hpp"
#include <type_traits>

template<typename T>
struct Vector2_t{
public:

    T x = T(0);
    T y = T(0);

    __fast_inline constexpr Vector2_t(){;}

<<<<<<< HEAD
    __fast_inline constexpr Vector2_t(const auto & _x, const auto & _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) {;}

=======
    // template <typename U>
    __fast_inline constexpr Vector2_t(const auto & _x, const auto & _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) {;}

    // template <typename U>
>>>>>>> oled_support
    __fast_inline constexpr Vector2_t(const Vector2_t<auto> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}


    __fast_inline constexpr Vector2_t<T> normalize(){*this /= this->length();}
    __fast_inline constexpr Vector2_t<T> normalized() const;
    __fast_inline constexpr T cross(const Vector2_t<T> & with) const;
    __fast_inline constexpr T dot(const Vector2_t<T> & with) const;
    __fast_inline constexpr Vector2_t<T> improduct(const Vector2_t<T> & b) const;
    __fast_inline constexpr Vector2_t<T> rotate(const T & r)const;
    __fast_inline constexpr Vector2_t<T> abs() const;
    __fast_inline T angle() const {return atan2f(y, x);}
    constexpr T angle_to(const Vector2_t<T> & to) const;
    constexpr T angle_to_point(const Vector2_t<T> & to) const;
    constexpr T aspect() const {return (!!y) ? x/y : T(0);}
    constexpr Vector2_t<T> bounce(const Vector2_t<T> &n) const;
    constexpr Vector2_t<T> ceil() const;

    constexpr Vector2_t<T> clampmin(const auto & length) const;
    constexpr Vector2_t<T> clampmax(const auto & length) const;


    constexpr Vector2_t<T> clamp(const auto & _min, const auto & _max) const;
    constexpr Vector2_t<T> dir_to(const Vector2_t<T> & b) const;
    constexpr T dist_to(const Vector2_t<T> & b) const;
    constexpr T dist_squared_to(const Vector2_t<T> & b) const;
    constexpr Vector2_t<T> floor() const;
    constexpr bool is_equal_approx(const Vector2_t<T> & v) const;


    constexpr bool has_point(const Vector2_t<auto> & _v)const;
    constexpr bool is_normalized() const {return (fabs(x*x + y*y + T(-1)) <= T(CMP_EPSILON));}
    constexpr __fast_inline T length() const {return sqrt(x*x + y*y);}
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

    constexpr __fast_inline Vector2_t<T> & operator-() const{
        x = -x;
        y = -y;
        return *this;
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

    constexpr  __no_inline explicit operator String() const{
        return (String('(') + String(static_cast<float>(x)) + String(", ") + String(static_cast<float>(y)) + String(')'));
    }

    __no_inline String toString(unsigned char decimalPlaces = 2){
        return (String('(') + String(static_cast<float>(x), decimalPlaces) + String(", ") + String(static_cast<float>(y), decimalPlaces) + String(')'));
    }
};

#include "vector2_t.tpp"

typedef Vector2_t<float> Vector2f;
typedef Vector2_t<int> Vector2i;
typedef Vector2_t<real_t> Vector2;

#endif

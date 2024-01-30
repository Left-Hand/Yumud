#ifndef __VECTOR2_HPP_

#define __VECTOR2_HPP_

#include "../../src/defines/comm_inc.h"
#include "../string/String.hpp"
#include "../real.hpp"
#include <type_traits>

template<typename T>
struct Vector2_t{
public:    

    T x = T(0);
    T y = T(0);

    __fast_inline explicit Vector2_t() : x(T()), y(T()) {;}
    __fast_inline explicit Vector2_t(const T & _x, const T & _y) : x(_x), y(_y) {;}

    template <typename U>
	__fast_inline Vector2_t(const U & _x,const U & _y) {
		x = static_cast<T>(_x);
		y = static_cast<T>(_y);
	}

    __fast_inline Vector2_t<T> normalize(){*this /= this->length();}
    __fast_inline Vector2_t<T> normalized() const;
    __fast_inline T cross(const Vector2_t<T> & with) const;
    __fast_inline T dot(const Vector2_t<T> & with) const;
    __fast_inline Vector2_t<T> improduct(const Vector2_t<T> & b) const;
    __fast_inline Vector2_t<T> rotate(const T & r)const;

    __fast_inline Vector2_t<T> abs() const;
    __fast_inline T angle() const {return atan2f(y, x);}
    T angle_to(const Vector2_t<T> & to) const;
    T angle_to_point(const Vector2_t<T> & to) const;
    T aspect() const {return (!!y) ? x/y : T(0);}
    Vector2_t<T> bounce(const Vector2_t<T> &n) const;
    Vector2_t<T> ceil() const;
    
    template<typename U>
    Vector2_t<T> clampmin(const U & length) const;
    
    template<typename U>
    Vector2_t<T> clampmax(const U & length) const;
    
    template<typename U>
    Vector2_t<T> clamp(const U & _min, const U & _max) const;

    Vector2_t<T> dir_to(const Vector2_t<T> & b) const;
    T dist_to(const Vector2_t<T> & b) const;
    T dist_squared_to(const Vector2_t<T> & b) const;
    Vector2_t<T> floor() const;
    bool is_equal_approx(const Vector2_t<T> & v) const;
    bool is_normalized() const {return (fabs(x*x + y*y + T(-1)) <= T(CMP_EPSILON));}
    __fast_inline T length() const {return sqrt(x*x + y*y);}
    __fast_inline T length_squared() const {return (x*x + y*y);}
    Vector2_t<T> lerp(const Vector2_t<T> & b, const T & t) const;
    Vector2_t<T> move_toward(const Vector2_t<T> & to, const T & delta) const;

    Vector2_t<T> posmod(const T & mod) const;
    Vector2_t<T> posmodv(const Vector2_t<T> & modv) const;
    Vector2_t<T> project(const Vector2_t<T> & b) const;
    Vector2_t<T> reflect(const Vector2_t<T> & n) const;
    Vector2_t<T> round() const;
    Vector2_t<T> sign() const;
    Vector2_t<T> slerp(const Vector2_t<T> & b, const T & t) const;
    Vector2_t<T> slide(const Vector2_t<T>  & n) const;
    Vector2_t<T> snapped(const Vector2_t<T> & by) const;

    template<typename U>
    __fast_inline Vector2_t<T> & operator+=(const Vector2_t<U> & b){
        x += static_cast<T>(b.x);
        y += static_cast<T>(b.y);
        return *this;
    }

    template<typename U>
    __fast_inline Vector2_t<T> & operator-=(const Vector2_t<U> & b){
        x -= static_cast<T>(b.x);
        y -= static_cast<T>(b.y);
        return *this;
    }

    __fast_inline Vector2_t<T> & operator-() const{
        x = -x;
        y = -y;
        return *this;
    }

    template<typename U>
    __fast_inline Vector2_t<T> & operator*=(const U & n){
        using CommonType = typename std::common_type<T, U>::type;
        x = static_cast<T>(static_cast<CommonType>(x) * n);
        y = static_cast<T>(static_cast<CommonType>(y) * n);
        return *this;
    }

    template<typename U>
    __fast_inline Vector2_t<T> & operator/=(const U & n){
        using CommonType = typename std::common_type<T, U>::type;
        x = static_cast<T>(static_cast<CommonType>(x) / n);
        y = static_cast<T>(static_cast<CommonType>(y) / n);
        return *this;
    }

    __no_inline explicit operator String() const{
        return (String('(') + String(static_cast<float>(x)) + String(", ") + String(static_cast<float>(y)) + String(')'));
    }

    __no_inline String toString(unsigned char decimalPlaces = 2){
        return (String('(') + String(static_cast<float>(x), decimalPlaces) + String(", ") + String(static_cast<float>(y), decimalPlaces) + String(')'));
    }
};

#include "Vector2.tpp"

#endif

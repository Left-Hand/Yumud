#pragma once

#include "core/stream/ostream.hpp"
#include "core/math/real.hpp"
#include "primitive/arithmetic/angular.hpp"

namespace ymd{

template<typename T>
struct Vec2;
template<typename T>
struct [[nodiscard]] UnitVec2 final{
    T x;
    T y;

    static constexpr UnitVec2<T> LEFT = UnitVec2<T>(-1, 0);
    static constexpr UnitVec2<T> RIGHT = UnitVec2<T>(1, 0);

    static constexpr UnitVec2<T> X = UnitVec2<T>(1, 0);
    static constexpr UnitVec2<T> Y = UnitVec2<T>(0, 1);
    static constexpr UnitVec2<T> NEG_X = UnitVec2<T>(-1, 0);
    static constexpr UnitVec2<T> NEG_Y = UnitVec2<T>(0, -1);
    static constexpr UnitVec2<T> AXES = UnitVec2<T>(0, 1);

    static constexpr UnitVec2<T> UP = UnitVec2<T>(0, 1);
    static constexpr UnitVec2<T> DOWN = UnitVec2<T>(0, -1);

    template<typename U>
    [[nodiscard]] __fast_inline static constexpr UnitVec2<T> from_angle(
        const Angular<U> angle
    ){
        const auto [s,c] = angle.sincos();
        return {static_cast<T>(c), static_cast<T>(s)};
    }

    constexpr T length() const {return 1;}
    constexpr T inv_length() const {return 1;}
    constexpr T length_squared() const {return 1;}

    constexpr UnitVec2<T> normalized() const { return *this; }
    constexpr UnitVec2<T> operator-() const { return UnitVec2<T>{-x, -y}; }
    constexpr UnitVec2<T> operator+() const { return UnitVec2<T>{x, y}; }

    template<typename U>
    constexpr Vec2<T> operator *(const U rhs) const { 
        return Vec2<T>{x * rhs, y * rhs};
    }

    template<typename U>
    constexpr Vec2<T> operator /(const U rhs) const { 
        const auto inv_rhs = 1 / rhs;
        return Vec2<T>{x * inv_rhs, y * inv_rhs};
    }

    constexpr T dot(const UnitVec2<T> & other) const {
        return dot(Vec2<T>(*this));
    }

    constexpr T dot(const Vec2<T> & other) const {
        return x * other.x + y * other.y;
    }

    constexpr Vec2<T> cross(const UnitVec2<T> & other) const {
        return Vec2<T>(*this).cross(Vec2<T>(other));
    }

    constexpr Vec2<T> cross(const Vec2<T> & other) const {
        return Vec2<T>(*this).cross(other);
    }


    template<typename U>
    [[nodiscard]] __fast_inline constexpr UnitVec2<T> rotated(const Angular<U> angle)const{
        static_assert(not std::is_integral_v<U>);
        const auto [_s,_c] = angle.sincos();
        const auto s = static_cast<T>(_s);
        const auto c = static_cast<T>(_c);
        return UnitVec2<T>(
            static_cast<T>(x*c - y*s), 
            static_cast<T>(x*s + y*c)
        );
    }
    constexpr explicit operator Vec2<T>() const { return Vec2<T>{x, y}; }
};

template<typename U, typename T>
static constexpr Vec2<T> operator *(const U lhs, const UnitVec2<T> rhs) { 
    return Vec2<T>{rhs.x * lhs, rhs.y * lhs};
}
}
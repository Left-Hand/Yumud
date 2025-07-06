#pragma once

#include "types/vectors/vector2/vector2.hpp"

namespace ymd{

template<arithmetic T>
struct Point2<T>{
    T x;
    T y;

    constexpr Point2(){;}

    constexpr Point2(const auto & _x, const auto & _y): x(T(_x)), y(T(_y)){;}

    template<arithmetic U = T>
    constexpr Point2(const std::tuple<U, U> & v) : x(std::get<0>(v)), y(std::get<1>(v)){;}

    template<arithmetic U = T>
    constexpr Point2(const Vector2<U> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    template<arithmetic U = T>
    constexpr Point2(const Vector2<U> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    operator Vector2<T>() const{ return Vector2<T>{this->x, this->y}};
};
}
#pragma once

#include "types/vector2/vector2_t.hpp"

namespace ymd{

template<arithmetic T>
struct Point2D_t<T>{
    T x;
    T y;

    constexpr Point2D_t(){;}

    constexpr Point2D_t(const auto & _x, const auto & _y): x(T(_x)), y(T(_y)){;}

    template<arithmetic U = T>
    constexpr Point2D_t(const std::tuple<U, U> & v) : x(std::get<0>(v)), y(std::get<1>(v)){;}

    template<arithmetic U = T>
    constexpr Point2D_t(const Vector2_t<U> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    template<arithmetic U = T>
    constexpr Point2D_t(const Vector2_t<U> & _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {;}

    operator Vector2_t<T>() const{ return Vector2_t<T>{this->x, this->y}};
};
}
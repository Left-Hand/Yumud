#pragma once

#include "types/vector2/vector2.hpp"

namespace ymd{
    
template <typename T>
class Bezier2D_t {
public:
    Vector2<T> p0;
    Vector2<T> p1;
    Vector2<T> p2;  // 控制点
public:

    // 构造函数
    Bezier2D_t(const Vector2<T>& _p0, const Vector2<T>& _p1, const Vector2<T>& _p2)
        : p0(_p0), p1(_p1), p2(_p2) {}

    Vector2<T> lerp(T t) const {
        T oneMinusT = 1 - t;
        return oneMinusT * oneMinusT * p0 +
               2 * oneMinusT * t * p1 +
               t * t * p2;
    }
};

}
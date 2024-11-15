#pragma once

#include "types/vector2/Vector2_t.hpp"

namespace ymd{
    
template <typename T>
class Bezier2D_t {
public:
    Vector2_t<T> p0;
    Vector2_t<T> p1;
    Vector2_t<T> p2;  // 控制点
public:

    // 构造函数
    Bezier2D_t(const Vector2_t<T>& _p0, const Vector2_t<T>& _p1, const Vector2_t<T>& _p2)
        : p0(_p0), p1(_p1), p2(_p2) {}

    Vector2_t<T> lerp(T t) const {
        T oneMinusT = 1 - t;
        return oneMinusT * oneMinusT * p0 +
               2 * oneMinusT * t * p1 +
               t * t * p2;
    }
};

}
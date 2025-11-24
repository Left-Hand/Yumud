#pragma once

#include "types/vectors/Vector2.hpp"
#include "types/vectors/Vector3.hpp"
#include "types/vectors/polar.hpp"
#include "types/vectors/spherical_coordinates.hpp"
#include "types/shapes/circle2.hpp"

namespace ymd::geometry { 



template<typename T>
requires (not std::is_integral_v<T>)
constexpr HeaplessVector<Vec2<T>, 2> compute_intersection_points(
    const Circle2<T>& circle_a, 
    const Circle2<T>& circle_b
) {
    using Container = HeaplessVector<Vec2<T>, 2>;
    // 卫语句1: 半径检查
    if (circle_a.radius < T(0) || circle_b.radius < T(0)) [[unlikely]] 
        return Container::from_empty();
        // PANIC();
    
    const Vec2<T>& p1 = circle_a.center;
    const Vec2<T>& p2 = circle_b.center;
    const T & r1 = circle_a.radius;
    const T & r2 = circle_b.radius;
    
    const Vec2<T> delta = p2 - p1;
    const T & dx = delta.x;
    const T & dy = delta.y;
    const T distance_squ = dx * dx + dy * dy;
    
    if(distance_squ > square(r1 + r2))
        return Container::from_empty();

    if(distance_squ < square(r1 - r2))
        return Container::from_empty();
    
    // a = (r1**2 - r2**2 + d**2) / (2 * d)
    const T d = std::sqrt(distance_squ);
    const T inv_d = T(1) / d;
    const T a = (r1 * r1 - r2 * r2 + distance_squ) * T(0.5) * inv_d;
    // h = math.sqrt(r1**2 - a**2)
    const T h = std::sqrt(r1 * r1 - a * a);
    // x0 = x1 + a * (x2 - x1) / d
    // y0 = y1 + a * (y2 - y1) / d
    const T x0 = p1.x + a * dx * inv_d;
    const T y0 = p1.y + a * dy * inv_d;
    // rx = -(y2 - y1) * (h / d)
    // ry = (x2 - x1) * (h / d)
    const T rx = -(dy) * (h * inv_d);
    const T ry = (dx) * (h * inv_d);
    // intersection1 = (x0 + rx, y0 + ry)
    // intersection2 = (x0 - rx, y0 - ry)
    // return intersection1, intersection2
    return Container(
        Vec2<T>(x0 + rx, y0 + ry),
        Vec2<T>(x0 - rx, y0 - ry)
    );
}

}
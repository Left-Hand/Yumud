#pragma once

#include "algebra/vectors/vec2.hpp"
#include "algebra/vectors/vec3.hpp"
#include "algebra/vectors/polar.hpp"
#include "algebra/vectors/spherical_coordinates.hpp"
#include "algebra/shapes/circle2.hpp"
#include "algebra/regions/ray2.hpp"

namespace ymd::geometry { 



template<typename T>
requires (not std::is_integral_v<T>)
constexpr HeaplessVector<math::Vec2<T>, 2> compute_intersection_points(
    const math::Circle2<T>& circle_a, 
    const math::Circle2<T>& circle_b
) {
    using Container = HeaplessVector<math::Vec2<T>, 2>;
    // 卫语句1: 半径检查
    if (circle_a.radius < T(0) || circle_b.radius < T(0)) [[unlikely]] 
        return Container::from_empty();
        // PANIC();
    
    const math::Vec2<T>& p1 = circle_a.center;
    const math::Vec2<T>& p2 = circle_b.center;
    const T & r1 = circle_a.radius;
    const T & r2 = circle_b.radius;
    
    const math::Vec2<T> delta = p2 - p1;
    const T & dx = delta.x;
    const T & dy = delta.y;
    const T distance_squ = dx * dx + dy * dy;
    
    if(distance_squ > math::square(r1 + r2))
        return Container::from_empty();

    if(distance_squ < math::square(r1 - r2))
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
        math::Vec2<T>(x0 + rx, y0 + ry),
        math::Vec2<T>(x0 - rx, y0 - ry)
    );
}

template<typename T>
requires (not std::is_integral_v<T>)
constexpr Option<math::Vec2<T>> compute_intersection_point(
    const math::Ray2<T> & ray,
    const math::Circle2<T> & circle
){
    const auto ray_center = ray.center;
    const auto ray_orientation = ray.orientation;
    
    // Vector from circle center to ray origin
    const math::Vec2<T> d = ray_center - circle.center;
    
    // Ray direction vector (assumed to be normalized)
    const math::Vec2<T> v = math::Vec2<T>::from_angle(ray_orientation);
    
    // Quadratic equation coefficients: at^2 + bt + c = 0
    const T a = v.dot(v);
    const T b = T(2) * d.dot(v);
    const T c = d.dot(d) - circle.radius * circle.radius;
    
    // Calculate discriminant
    const T discriminant = b * b - T(4) * a * c;
    
    // No intersection if discriminant is negative
    if (discriminant < T(0)) {
        return None;
    }
    
    // Calculate the nearest intersection point
    const T sqrt_discriminant = std::sqrt(discriminant);
    const T inv2a = T(0.5) / (a);
    const T t1 = (-b - sqrt_discriminant) * inv2a;
    const T t2 = (-b + sqrt_discriminant) * inv2a;
    
    // For a ray, we only consider positive t values
    const T t = (t1 >= T(0)) ? t1 : t2;
    
    // If both t values are negative, no intersection in ray direction
    if (t < T(0)) {
        return None;
    }
    
    // Calculate intersection point
    return Some(ray_center + v * t);
}

}
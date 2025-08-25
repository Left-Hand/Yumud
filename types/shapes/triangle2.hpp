#pragma once

#include "prelude.hpp"

namespace ymd{

template<typename T>
struct Triangle2{
    std::array<Vec2<T>, 3> points;


    constexpr Vec2<T> top() const {
        const auto min_y = MIN(points[0].y, points[1].y, points[2].y);
        for (const auto& point : points) {
            if (point.y == min_y) {
                return point;
            }
        }
        return points[0]; // fallback
    }

    constexpr Vec2<T> bottom() const {
        const auto max_y = MAX(points[0].y, points[1].y, points[2].y);
        for (const auto& point : points) {
            if (point.y == max_y) {
                return point;
            }
        }
        return points[0]; // fallback
    }

    constexpr Vec2<T> middle() const {
        const auto y0 = points[0].y, y1 = points[1].y, y2 = points[2].y;
        const auto min_y = MIN(y0, y1, y2);
        const auto max_y = MAX(y0, y1, y2);
        
        if (y0 != min_y && y0 != max_y) return points[0];
        if (y1 != min_y && y1 != max_y) return points[1];
        if (y2 != min_y && y2 != max_y) return points[2];
        
        // If all points have same y or only two distinct y values, return one of the non-extreme points
        if (y0 == min_y && y1 == max_y) return points[2];
        if (y0 == max_y && y1 == min_y) return points[2];
        return points[1];
    }

    constexpr bool is_clockwise() const{
        return (points[1] - points[0]).cross(points[2] - points[0]) < 0;
    }

    constexpr bool is_counter_clockwise() const {
        return (points[1] - points[0]).cross(points[2] - points[0]) > 0;
    }

    constexpr T area() const {
        return (points[1] - points[0]).cross(points[2] - points[0]) / 2;
    }

    constexpr bool contains_point(const Vec2<T>& p) const {
        const auto& a = points[0];
        const auto& b = points[1];
        const auto& c = points[2];
        
        // 计算重心坐标
        const Vec2<T> v0 = b - a;
        const Vec2<T> v1 = c - a;
        const Vec2<T> v2 = p - a;
        
        const T dot00 = v0.dot(v0);
        const T dot01 = v0.dot(v1);
        const T dot02 = v0.dot(v2);
        const T dot11 = v1.dot(v1);
        const T dot12 = v1.dot(v2);
        
        // 计算重心坐标参数
        const T inv_denom = 1 / (dot00 * dot11 - dot01 * dot01);
        const T u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
        const T v = (dot00 * dot12 - dot01 * dot02) * inv_denom;
        
        // 检查点是否在三角形内部
        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }

    constexpr Triangle2<T> to_sorted_by_y() const {
        Triangle2<T> result = *this;
        auto& p = result.points; // 引用别名提高可读性
        
        // 最优的比较网络 for 3 elements
        if (p[0].y > p[1].y) std::swap(p[0], p[1]);
        if (p[1].y > p[2].y) std::swap(p[1], p[2]);
        if (p[0].y > p[1].y) std::swap(p[0], p[1]);
        
        return result;
    }

    constexpr Rect2<T> bounding_box() const {
        return Rect2<T>::from_minimal_bounding_box(points);
    }

    constexpr Vec2<T> operator[](const size_t index) const{
        return points[index];
    }

    constexpr Vec2<T> & operator[](const size_t index){
        return points[index];
    }

    constexpr Vec2<T> compute_centroid() const{
        return (points[0] + points[1] + points[2]) / 3;
    }

    constexpr std::tuple<T, T, T> compute_barycentric_2d(const Vec2<T> p)
    {
        const auto [xp, yp] = p;
        const T xa = points[0].x, ya = points[0].y;
        const T xb = points[1].x, yb = points[1].y;
        const T xc = points[2].x, yc = points[2].y;
        const T gamma = ((xb - xa) * (yp - ya) - (xp - xa) * (yb - ya)) / 
                    ((xb - xa) * (yc - ya) - (xc - xa) * (yb - ya));
        const T beta = (xp - xa - gamma * (xc - xa)) / (xb - xa);
        const T alpha = 1 - beta - gamma;
        return {alpha,beta,gamma};
    }
};


template <typename T>
struct is_placed_t<Triangle2<T>>: std::true_type {};
}
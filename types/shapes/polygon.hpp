#pragma once

#include "types/shapes/prelude.hpp"
#include "types/regions/rect2.hpp"

namespace ymd{

template<typename T, size_t N = std::dynamic_extent>
struct Polygon final {
    std::span<const Vec2<T>, N> points;
    constexpr explicit Polygon(std::span<const Vec2<T>, N> points) : points(points) {}

    constexpr bool is_convex() const{
        if (points.size() < 3) {
            return true; // Triangles and simpler shapes are always convex
        }
        
        auto cross_product = [](const Vec2<T>& a, const Vec2<T>& b, const Vec2<T>& c) -> T {
            Vec2<T> ab = b - a;
            Vec2<T> bc = c - b;
            return ab.x() * bc.y() - ab.y() * bc.x();
        };
        
        bool sign = cross_product(points[points.size()-1], points[0], points[1]) >= 0;
        
        for (size_t i = 0; i < points.size(); ++i) {
            size_t next = (i + 1) % points.size();
            size_t next_next = (i + 2) % points.size();
            
            T cp = cross_product(points[i], points[next], points[next_next]);
            bool current_sign = cp >= 0;
            
            if (current_sign != sign) {
                return false;
            }
        }
        
        return true;
    }

    constexpr bool is_clockwise() const{
        if (points.size() < 3) {
            return false; // Not enough points to determine orientation
        }
        
        // Calculate the signed area using the shoelace formula
        T area = T{0};
        for (size_t i = 0; i < points.size(); ++i) {
            size_t next = (i + 1) % points.size();
            area += points[i].x() * points[next].y() - points[next].x() * points[i].y();
        }
        
        // If area is negative, the polygon is clockwise
        return area < T{0};
    }

    constexpr Vec2<T> center() const {
        const T inv_s = T{1} / static_cast<T>(points.size());

        Vec2 sum = Vec2<T>::ZERO;
        for(const auto& point : points){
            sum += point * inv_s;
        }
        return sum;
    }

    constexpr Rect2<T> to_bounding_box() const {
        return Rect2<T>::from_minimal_bounding_box(points);

    }
};

template<typename T, size_t N>
struct is_placed_t<Polygon<T, N>> : std::true_type {};

}
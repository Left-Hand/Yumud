#pragma once

#include "algebra/vectors/vec2.hpp"

namespace ymd{
    
template <typename T>
class Bezier2 {
public:
    math::Vec2<T> p0;  // 起始点
    math::Vec2<T> p1;  // 控制点
    math::Vec2<T> p2;  // 结束点

public:
    // 构造函数
    constexpr explicit Bezier2(
        const math::Vec2<T>& _p0, 
        const math::Vec2<T>& _p1, 
        const math::Vec2<T>& _p2
    )
        : p0(_p0), p1(_p1), p2(_p2) {}


    // 在t处计算曲线上的点 (t ∈ [0, 1])
    constexpr math::Vec2<T> evaluate(T t) const {
        T oneMinusT = 1 - t;
        return oneMinusT * oneMinusT * p0 + 
               2 * oneMinusT * t * p1 + 
               t * t * p2;
    }

    // 计算t处的切线向量（一阶导数）
    constexpr math::Vec2<T> tangent(T t) const {
        return 2 * (1 - t) * (p1 - p0) + 2 * t * (p2 - p1);
    }

    // 计算t处的曲率
    constexpr T curvature(T t) const {
        math::Vec2<T> d1 = tangent(t);
        math::Vec2<T> d2 = 2 * (p2 - 2 * p1 + p0);  // 二阶导数
        
        T crossProduct = d1.x * d2.y - d1.y * d2.x;
        T d1Length = std::sqrt(d1.x * d1.x + d1.y * d1.y);
        
        return crossProduct / (d1Length * d1Length * d1Length);
    }

    // 将曲线分割为两部分
    constexpr std::pair<Bezier2<T>, Bezier2<T>> split(T t) const {
        math::Vec2<T> _q0 = p0 + t * (p1 - p0);
        math::Vec2<T> _q1 = p1 + t * (p2 - p1);
        math::Vec2<T> r = _q0 + t * (_q1 - _q0);

        return {
            Bezier2<T>(p0, _q0, r),  // 第一段曲线
            Bezier2<T>(r, _q1, p2)   // 第二段曲线
        };
    }

    // 计算曲线的近似长度（通过采样）
    constexpr T approximate_length(size_t samples) const {
        if (samples < 2) samples = 2;
        
        T length = 0;
        math::Vec2<T> prev = p0;
        
        for (size_t i = 1; i <= samples; ++i) {
            T t = static_cast<T>(i) / samples;
            math::Vec2<T> current = evaluate(t);
            length += (current - prev).length();
            prev = current;
        }
        
        return length;
    }

    // 获取曲线的包围盒
    constexpr math::Rect2<T> bounding_box() const {
        math::Vec2<T> min, max;
        min.x = std::min(std::min(p0.x, p1.x), p2.x);
        min.y = std::min(std::min(p0.y, p1.y), p2.y);
        max.x = std::max(std::max(p0.x, p1.x), p2.x);
        max.y = std::max(std::max(p0.y, p1.y), p2.y);

        return math::Rect2<T>::from_corners(min, max);
    }


    // 检查点是否在曲线上（带容差）
    constexpr bool is_oncurve(const math::Vec2<T> point, T tolerance) const {
        // 简单实现：检查点与曲线上最近点的距离
        // 更精确的实现可能需要解方程
        const size_t samples = 20;
        for (size_t i = 0; i <= samples; ++i) {
            T t = static_cast<T>(i) / samples;
            if ((evaluate(t) - point).lengthSquared() <= tolerance * tolerance) {
                return true;
            }
        }
        return false;
    }

    // 与 lerp 函数保持兼容
    constexpr math::Vec2<T> lerp(T t) const {
        return evaluate(t);
    }
};

}
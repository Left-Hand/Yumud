#pragma once

#include "algebra/vectors/vec2.hpp"

namespace ymd::geometry { 

template<typename T>
struct [[nodiscard]] BSpline final{
    using Vec = Vec2<T>;
    using ControlPoints = std::span<Vec>;
    

    ControlPoints control_points;
    int degree = 3; // Default cubic B-spline

    // 基函数计算 (Cox-de Boor递归算法)
    // 参数说明:
    // i: 基函数索引
    // p: 基函数阶数(次数)
    // t: 参数值
    // knots: 节点向量
    constexpr [[nodiscard]] T basis_function(size_t i, int p, T t, const std::span<const T> knots) const {
        // 0次基函数(递归基础情况)
        // 如果参数t在第i个节点区间内，返回1，否则返回0
        if (p == 0) {
            return (knots[i] <= t && t < knots[i + 1]) ? T(1) : T(0);
        }
        
        // 计算左侧递归项的系数
        T left_denom = knots[i + p] - knots[i];  // 分母
        T left = 0;
        // 避免除零错误，如果分母不为零则计算左侧项
        if (left_denom != T(0)) {
            left = (t - knots[i]) / left_denom * basis_function(i, p - 1, t, knots);
        }
        
        // 计算右侧递归项的系数
        T right_denom = knots[i + p + 1] - knots[i + 1];  // 分母
        T right = 0;
        // 避免除零错误，如果分母不为零则计算右侧项
        if (right_denom != T(0)) {
            right = (knots[i + p + 1] - t) / right_denom * basis_function(i + 1, p - 1, t, knots);
        }
        
        // 返回左右两项之和，这是B样条基函数的递归定义
        return left + right;
    }
    
    // 节点向量生成器迭代器
    struct [[nodiscard]] UniformKnotsIterator {
    private:
        const BSpline& spline;
        size_t n;           // 控制点数量-1
        size_t m;           // 节点向量长度参数
        size_t current_idx; // 当前索引
        bool is_empty;      // 是否为空
        
    public:
        // 构造函数
        constexpr explicit UniformKnotsIterator(const BSpline& _spline) 
            : spline(_spline), current_idx(0), is_empty(_spline.control_points.empty()) {
            if (!is_empty) {
                n = _spline.control_points.size() - 1;
                m = n + _spline.degree + 1;
            } else {
                current_idx = 0;
            }
        }
        
        // 检查是否还有下一个节点值
        constexpr [[nodiscard]] bool has_next() const {
            if (is_empty) return false;
            return current_idx <= m;
        }
        
        // 获取下一个节点值
        constexpr T next() {
            if (!has_next()) {
                return T(0); // 或抛出异常
            }
            
            size_t i = current_idx++;
            
            // 在开始处的clamped节点
            if (i <= static_cast<size_t>(spline.degree)) {
                return T(0);
            }
            // 中间均匀分布的节点
            else if (i <= n) {
                return static_cast<T>(i - spline.degree) / static_cast<T>(n - spline.degree + 1);
            }
            // 结束处的clamped节点
            else {
                return T(1);
            }
        }
    };
    
    // 返回节点向量迭代器
    constexpr UniformKnotsIterator uniform_knots_iter() const {
        return UniformKnotsIterator(*this);
    }
    
    
    // 在参数t处计算曲线上的点 [t ∈ [0, 1]]
    constexpr Vec evaluate(const T t) const {
        if (t < 0) PANIC{};
        if (t > 1) PANIC{};

        // 获取均匀节点向量
        auto knots = uniform_knots();
        // 初始化结果为零向量
        Vec result = Vec::ZERO;
        
        // 根据B样条曲线定义计算曲线上一点
        // C(t) = Σ(Ni,p(t) * Pi) 其中Ni,p是第i个p次基函数，Pi是第i个控制点
        for (size_t i = 0; i < control_points.size(); ++i) {
            // 计算第i个基函数在参数t处的值
            T basis = basis_function(i, degree, t, knots);
            // 累加 基函数值 × 控制点
            result += control_points[i] * basis;
        }
        
        return result;
    }

    // 对曲线进行细分，生成一系列点
    // num_segments: 细分段数
    struct [[nodiscard]] TessellationIterator {
    public:
        const BSpline& spline;
        size_t num_segments;
        size_t idx = 0;

        // 检查是否还有下一个点
        [[nodiscard]] constexpr bool has_next() const {
            return idx <= num_segments;
        }
        
        // 获取下一个点
        constexpr Vec next() {
            if (idx > num_segments) {
                PANIC{};
            }
            
            T t = (num_segments > 0) ? static_cast<T>(idx) / num_segments : T(0);
            ++idx;
            return spline.evaluate(t);
        }
    };
    
    // 返回迭代器，用于遍历细分点
    // num_segments: 细分段数
    constexpr TessellationIterator tessellate_iter(size_t num_segments) const {
        return TessellationIterator(*this, num_segments, 0);
    }
    
    // 计算参数t处的导数(切线向量)
    constexpr Vec derivative(T t) const {

        // 将参数t限制在[0,1]范围内
        t = std::clamp(t, T(0), T(1));
        
        // 获取均匀节点向量
        auto knots = uniform_knots();
        // 初始化结果为零向量
        Vec result = Vec::ZERO;
        
        // B样条曲线导数计算
        // 导数公式: C'(t) = Σ( Ni,p-1(t) * (p/(knots[i+p]-knots[i])) * (Pi+1-Pi) )
        for (size_t i = 0; i < control_points.size() - 1; ++i) {
            // 计算节点差值作为分母
            T denom = knots[i + degree + 1] - knots[i + 1];
            // 如果分母不为零，则计算该项贡献
            if (denom != T(0)) {
                // 计算导数系数
                T coeff = static_cast<T>(degree) / denom;
                // 计算相邻控制点的差值
                Vec diff = control_points[i + 1] - control_points[i];
                // 累加导数贡献: 系数 × 控制点差值 × 基函数值
                result += diff * coeff * basis_function(i, degree - 1, t, knots);
            }
        }
        
        return result;
    }
};
}
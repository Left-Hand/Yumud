#pragma once

#include "core/math/realmath.hpp"
#include <span>

namespace ymd::statistics {

template<typename T>
struct LinearRegression {
    T k;
    T b;

    [[nodiscard]] constexpr T operator()(T x) const noexcept {
        return k * x + b;
    }

    [[nodiscard]] static constexpr LinearRegression from_x_and_y(
        std::span<const T> x, 
        std::span<const T> y
    ) noexcept {
        if(x.size() != y.size()) [[unlikely]]
            __builtin_abort();

        const size_t n = x.size();
        
        // 边界情况处理
        if(n == 0) [[unlikely]] return {0, 0};
        if(n == 1) [[unlikely]] return {0, y[0]};

        T sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
        
        for(size_t i = 0; i < n; ++i) {
            const T xi = x[i];
            const T yi = y[i];
            sum_x += xi;
            sum_y += yi;
            sum_xy += xi * yi;
            sum_xx += xi * xi;
        }

        // 计算回归参数
        const T denominator = n * sum_xx - sum_x * sum_x;
        
        if(std::abs(denominator) < 1e-10f) [[unlikely]] {
            return {0.0f, sum_y / n};
        }

        const T k_val = (n * sum_xy - sum_x * sum_y) / denominator;
        const T b_val = (sum_y - k_val * sum_x) / n;

        return LinearRegression{
            .k = k_val, 
            .b = b_val
        };
    }
};
}
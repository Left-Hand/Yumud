#pragma once

#include "core/math/matrix/static_matrix.hpp"
#include <cmath>

namespace ymd{
namespace details{
template<typename T, size_t R, size_t C>
struct JacobiSVD {
private:
    static constexpr size_t K = MIN(R, C);  // 奇异值数量
    static constexpr T epsilon = static_cast<T>(1e-6);
public:
    struct Solution{
        math::Matrix<T, R, R> U;
        math::Matrix<T, K, 1> sigma;
        math::Matrix<T, C, C> V;
    };

    explicit constexpr JacobiSVD(const math::Matrix<T, R, C> & matrix, size_t max_iterations) {
        compute(matrix, max_iterations);
    }
    
    /**
     * 计算矩阵的SVD分解
     * 使用双边雅可比方法：通过左右旋转同时对角化矩阵
     */
    constexpr void compute(const math::Matrix<T, R, C>& matrix, size_t max_iterations) {
        // 初始化U为单位矩阵，V为单位矩阵，B为输入矩阵
        math::Matrix<T, R, C> B = matrix;
        
        // T epsilon = std::sqrt(std::numeric_limits<T>::epsilon());

        bool converged = false;
        
        // 主迭代循环
        for (size_t iter = 0; iter < max_iterations && !converged; iter++) {
            converged = true;
            T max_off_diag = 0;
            
            // 遍历所有可能的(p,q)对
            for (size_t p = 0; p < K; p++) {
                for (size_t q = p + 1; q < K; q++) {
                    // 计算2x2子矩阵的范数
                    const T app = B(p, p), apq = B(p, q);
                    const T aqp = B(q, p), aqq = B(q, q);
                    
                    const T off_diag = std::sqrt(apq * apq + aqp * aqp);
                    max_off_diag = std::max(max_off_diag, off_diag);
                    
                    // 如果非对角元素足够小，跳过
                    if (off_diag < epsilon * (std::abs(app) + std::abs(aqq))) {
                        continue;
                    }
                    
                    converged = false;
                    
                    // 计算2x2子矩阵的SVD旋转参数
                    auto [c_left, s_left, c_right, s_right] = 
                        compute2x2_svd_rotation(B, p, q);
                    
                    // 应用双边旋转
                    apply_bilateral_rotation(B, U_, V_, p, q, c_left, s_left, c_right, s_right);
                }
            }
            
            // 检查收敛条件
            if (max_off_diag < epsilon) {
                break;
            }
        }
        
        // 提取奇异值（B矩阵的对角线元素绝对值）
        for (size_t i = 0; i < K; i++) {
            sigma_.at(i, 0) = std::abs(B(i, i));
        }
        
        // 确保奇异值为非负，并调整符号
        adjust_signs(B, sigma_);
        
        // 对奇异值进行降序排序，并相应调整U和V
        sort_singular_values(B);
        
        is_computed_ = true;
    }
    
    // 获取结果矩阵
    constexpr Option<Solution> solution() const {
        if(not is_computed_) return None;
        return Some(Solution{U_, sigma_, V_});
    }

    constexpr bool is_computed() const { return is_computed_; }
private:
    /**
     * 计算2x2矩阵的SVD旋转参数
     * 返回: (左旋转余弦, 左旋转正弦, 右旋转余弦, 右旋转正弦)
     */
    static constexpr std::tuple<T, T, T, T> compute2x2_svd_rotation(
        const math::Matrix<T, R, C>& B, size_t p, size_t q) {
        
        // 提取2x2子矩阵元素
        T a00 = B(p, p), a01 = B(p, q);
        T a10 = B(q, p), a11 = B(q, q);
        
        // T epsilon = std::numeric_limits<T>::epsilon();
        
        // 处理接近对角的情况
        if (std::abs(a01) < epsilon && std::abs(a10) < epsilon) {
            return {1, 0, 1, 0};
        }
        
        // 计算对称矩阵的元素
        T m00 = a00 * a00 + a10 * a10;  // (A^T A)_{pp}
        T m11 = a01 * a01 + a11 * a11;  // (A^T A)_{qq}  
        T m01 = a00 * a01 + a10 * a11;  // (A^T A)_{pq}
        
        // 计算右旋转（用于对角化A^T A）
        T m_tau = (m11 - m00) / (2 * m01);
        T t;
        if (m_tau >= 0) {
            t = static_cast<T>(1.0) / (m_tau + std::sqrt(1 + m_tau * m_tau));
        } else {
            t = static_cast<T>(-1.0) / (-m_tau + std::sqrt(1 + m_tau * m_tau));
        }
        
        T c_right = static_cast<T>(1.0) / std::sqrt(1 + t * t);
        T s_right = t * c_right;
        
        // 计算左旋转（用于对角化A A^T）
        // 应用右旋转后计算左旋转
        T y00 = a00 * c_right - a01 * s_right;
        T y10 = a10 * c_right - a11 * s_right;
        
        T norm = std::sqrt(y00 * y00 + y10 * y10);
        T c_left = static_cast<T>(1.0);
        T s_left = static_cast<T>(0.0);
        if (norm > epsilon) {
            c_left = y00 / norm;
            s_left = y10 / norm;
        }
        
        return {c_left, s_left, c_right, s_right};
    }
    
    /**
     * 应用双边雅可比旋转
     */
    static constexpr void apply_bilateral_rotation(
        math::Matrix<T, R, C>& B, 
        math::Matrix<T, R, R>& U,
        math::Matrix<T, C, C>& V,
        size_t p, size_t q,
        T c_left, T s_left, T c_right, T s_right) {
        
        // 应用左旋转到B的行（影响U）
        for (size_t j = 0; j < C; j++) {
            T bpj = B(p, j);
            T bqj = B(q, j);
            B(p, j) = c_left * bpj + s_left * bqj;
            B(q, j) = -s_left * bpj + c_left * bqj;
        }
        
        // 应用右旋转到B的列（影响V）
        for (size_t i = 0; i < R; i++) {
            T bip = B(i, p);
            T biq = B(i, q);
            B(i, p) = c_right * bip - s_right * biq;
            B(i, q) = s_right * bip + c_right * biq;
        }
        
        // 更新左奇异向量U
        for (size_t i = 0; i < R; i++) {
            T uip = U(i, p);
            T uiq = U(i, q);
            U(i, p) = c_left * uip + s_left * uiq;
            U(i, q) = -s_left * uip + c_left * uiq;
        }
        
        // 更新右奇异向量V
        for (size_t i = 0; i < C; i++) {
            T vip = V(i, p);
            T viq = V(i, q);
            V(i, p) = c_right * vip - s_right * viq;
            V(i, q) = s_right * vip + c_right * viq;
        }
    }
    
    /**
     * 调整符号以确保奇异值为非负
     */
    constexpr void adjust_signs(math::Matrix<T, R, C>& B, math::Matrix<T, K, 1>& sigma) {
        for (size_t i = 0; i < K; i++) {
            if (B(i, i) < 0) {
                sigma.at(i, 0) = -B(i, i);
                // 调整U的对应列符号
                for (size_t j = 0; j < R; j++) {
                    U_.at(j, i) = -U_.at(j, i);
                }
            } else {
                sigma.at(i, 0) = B(i, i);
            }
        }
    }
    
    /**
     * 对奇异值进行排序（降序）
     */
    constexpr void sort_singular_values(math::Matrix<T, R, C>& B) {
        for (size_t i = 0; i < K; i++) {
            size_t max_index = i;
            T max_value = sigma_.at(i, 0);
            
            // 找到第i大的奇异值
            for (size_t j = i + 1; j < K; j++) {
                if (sigma_.at(j, 0) > max_value) {
                    max_value = sigma_.at(j, 0);
                    max_index = j;
                }
            }
            
            // 如果需要交换
            if (max_index != i) {
                // 交换奇异值
                std::swap(sigma_.at(i, 0), sigma_.at(max_index, 0));
                
                // 交换U的对应列
                for (size_t k = 0; k < R; k++) {
                    std::swap(U_.at(k, i), U_.at(k, max_index));
                }
                
                // 交换V的对应列
                for (size_t k = 0; k < C; k++) {
                    std::swap(V_.at(k, i), V_.at(k, max_index));
                }
            }
        }
    }

private:
    math::Matrix<T, R, R> U_          = math::Matrix<T, R, R>::identity();      // 左奇异向量矩阵
    math::Matrix<T, K, 1> sigma_      = math::Matrix<T, K, 1>::zero();  // 奇异值向量
    math::Matrix<T, C, C> V_          = math::Matrix<T, C, C>::identity();      // 右奇异向量矩阵


    bool is_computed_ = false;        // 计算完成标志
};
}
template<typename T, size_t R, size_t C, typename S = details::JacobiSVD<T, R, C>::Solution>
Option<S> solve_jacobi_svd(const math::Matrix<T, R, C> & matrix, size_t max_iterations) {
    return details::JacobiSVD<T, R, C>(matrix, max_iterations).solution();
}

}
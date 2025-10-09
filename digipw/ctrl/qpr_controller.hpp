#pragma once

#include "dsp/z_transformation.hpp"

namespace ymd::dsp{

    
// QPR控制器
// https://github.com/SSC202/Three_Port_ACDC_Converter/blob/main/Software/Controller/my_math.c

template<typename T>
class SecondOrderSystem {
public:

    // 直接使用传递函数对象构造
    constexpr explicit SecondOrderSystem(const Z_TransferCoefficients<T, 3, 3>& tf)
        : tf_(tf) {
        reset();
    }

    constexpr T operator()(const T err) {
        // 更新输入序列
        u_[2] = u_[1];
        u_[1] = u_[0];
        u_[0] = err;

        // 使用传递函数系数计算输出
        // y[n] = b0*u[n] + b1*u[n-1] + b2*u[n-2] - a1*y[n-1] - a2*y[n-2]
        // 注意：tf_.den[0] = 1.0，所以不需要除以a0
        y_[0] = tf_.num[0] * u_[0] + tf_.num[1] * u_[1] + tf_.num[2] * u_[2]
               - tf_.den[1] * y_[1] - tf_.den[2] * y_[2];

        // 更新输出序列
        output_ = y_[0];
        y_[2] = y_[1];
        y_[1] = y_[0];

        return output_;
    }

    constexpr void reset() {
        output_ = T(0);
        u_[0] = u_[1] = u_[2] = T(0);
        y_[0] = y_[1] = y_[2] = T(0);
    }

    // 获取传递函数对象
    [[nodiscard]] constexpr const Z_TransferCoefficients<T, 3, 3>& transfer_function() const { return tf_; }
    
    // 获取当前输出
    [[nodiscard]] constexpr T output() const { return output_; }


private:
    Z_TransferCoefficients<T, 3, 3> tf_;  // 传递函数系数
    T u_[3] = {T(0), T(0), T(0)};  // 输入序列: u[0]=当前, u[1]=前一次, u[2]=前两次
    T y_[3] = {T(0), T(0), T(0)};  // 输出序列: y[0]=当前, y[1]=前一次, y[2]=前两次  
    T output_ = T(0);               // 当前输出值
};


template<typename T>
struct QPRControllerConfig {
    T kp;        // 直流增益
    T kr;        // 谐振增益  
    T wr;        // 谐振频率
    T wc;        // 谐振峰带宽
    T t_sample;  // 采样时间

    constexpr Z_TransferCoefficients<T, 3, 3> to_coeffs() const {
        const T d0 = (t_sample * t_sample * wr * wr) + (4 * wc * t_sample) + 4;
        const T inv_d0 = 1 / d0;

        const T a1 = (2 * t_sample * t_sample * wr * wr - 8) * inv_d0;
        const T a2 = (t_sample * t_sample * wr * wr - 4 * wc * t_sample + 4) * inv_d0;

        const T b0 = (
            4 * kp + t_sample * t_sample * kp * wr * wr + 
            4 * t_sample * kp * wc + 4 * t_sample * kr * wc) * inv_d0;

        const T b1 = (
            2 * kp * t_sample * t_sample * wr * wr - 8 * kp) * inv_d0;

        const T b2 = (
            4 * kp + t_sample * t_sample * kp * wr * wr - 
            4 * t_sample * kp * wc - 4 * t_sample * kr * wc) * inv_d0;

        // 注意：分母系数需要包含 a0 = 1.0
        return Z_TransferCoefficients<T, 3, 3>(
            std::array<T, 3>{b0, b1, b2},        // 分子系数 [b0, b1, b2]
            std::array<T, 3>{T(1.0), a1, a2}     // 分母系数 [a0=1, a1, a2]
        );
    }

    #if 0

    // 便捷函数：创建多谐振点QPR控制器
    constexpr auto create_multi_resonant_qpr(
        T kp, T kr, T t_sample, 
        std::initializer_list<T> resonant_freqs, T bandwidth) {
        
        
        if (resonant_freqs.size() == 0) {
            // 如果没有谐振频率，创建纯比例控制器
            return SecondOrderSystem<T>(QPRControllerConfig{kp, T(0), T(0), bandwidth, t_sample}.to_coeffs());
        }
        
        // 为每个谐振频率创建QPR控制器并并联
        auto first_freq = resonant_freqs.begin();
        auto controller = SecondOrderSystem<T>(
            QPRControllerConfig{kp, kr, *first_freq, bandwidth, t_sample}.to_coeffs();
        );
        
        for (auto it = first_freq + 1; it != resonant_freqs.end(); ++it) {
            auto next_controller = SecondOrderSystem<T>(QPRControllerConfig{T(0), kr, *it, bandwidth, t_sample});
            controller = SecondOrderSystem<T>(controller.parallel(next_controller));
        }
        
        return controller;
    }

    #endif
};

}
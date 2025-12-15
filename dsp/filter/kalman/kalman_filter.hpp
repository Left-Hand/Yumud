#pragma once

#include "core/math/matrix/static_matrix.hpp"
#include "core/utils/Result.hpp"

namespace ymd::dsp::kalman{


struct Linear{};

// 策略类：状态转移函数
template<typename Policy, typename T, size_t N_STATES, size_t N_INPUTS>
struct StateTransition;

// 策略类：观测函数
template<typename Policy, typename T, size_t N_STATES, size_t N_MEAS>
struct Observation;

// 策略类：过程噪声协方差更新
template<typename Policy, typename T, size_t N_STATES>
struct ProcessNoise;

// 策略类：测量噪声协方差更新
template<typename Policy, typename T, size_t N_MEAS>
struct MeasurementNoise;



// 策略类：状态转移函数
template<typename T, size_t N_STATES, size_t N_INPUTS>
struct StateTransition<Linear, T, N_STATES, N_INPUTS>{
    constexpr Matrix<T, N_STATES, 1> operator()(
        const Matrix<T, N_STATES, 1> & x,
        const Matrix<T, N_INPUTS, 1> & u,
        const Matrix<T, N_STATES, N_STATES> & F
    ) const {
        return F * x + u; // 默认线性模型
    }
};

// 策略类：观测函数
template<typename T, size_t N_STATES, size_t N_MEAS>
struct Observation<Linear, T, N_STATES, N_MEAS> {
    constexpr Matrix<T, N_MEAS, 1> operator()(
        const Matrix<T, N_STATES, 1>& x,
        const Matrix<T, N_MEAS, N_STATES>& H
    ) const {
        return H * x; // 默认线性观测
    }
};

// 策略类：过程噪声协方差更新
template<typename T, size_t N_STATES>
struct ProcessNoise<Linear, T, N_STATES>  {
    constexpr Matrix<T, N_STATES, N_STATES> operator()(
        const Matrix<T, N_STATES, N_STATES>& Q,
        T dt
    ) const {
        return Q; // 默认恒定过程噪声
    }
};

// 策略类：测量噪声协方差更新
template<typename T, size_t N_MEAS>
struct MeasurementNoise<Linear, T, N_MEAS>  {
    constexpr Matrix<T, N_MEAS, N_MEAS> operator()(
        const Matrix<T, N_MEAS, N_MEAS>& R,
        const Matrix<T, N_MEAS, 1>& z
    ) const {
        return R; // 默认恒定测量噪声
    }
};

// 通用的Kalman滤波器
template<
    typename T,
    size_t N_STATES,
    size_t N_INPUTS,
    size_t N_MEAS,
    typename StateTransition,
    typename Observation,
    typename ProcessNoise,
    typename MeasurementNoise
>
class KalmanFilter {
public:

    // 添加参数验证
    static_assert(N_STATES > 0, "Number of states must be positive");
    static_assert(N_MEAS > 0, "Number of measurements must be positive");


    enum class Contracts:uint8_t{
        CovarianceIsNotSymmetric,
        ProcessNoiseIsNotSymmetric,
        MeasurementNoiseIsNotSymmetric
    };

    friend OutputStream & operator << (OutputStream & os, const Contracts & cts){
        switch (cts) {
            case Contracts::CovarianceIsNotSymmetric: 
                return os << "covariance is not symmetric";
            case Contracts::ProcessNoiseIsNotSymmetric: 
                return os << "process noise is not symmetric";
            case Contracts::MeasurementNoiseIsNotSymmetric: 
                return os << "measurement noise is not symmetric";
        }
        __builtin_unreachable();
    }

    struct Config{
        Matrix<T, N_STATES, 1>          initial_state;
        Matrix<T, N_STATES, N_STATES>   initial_covariance;
        Matrix<T, N_STATES, N_STATES>   F;
        Matrix<T, N_STATES, N_INPUTS>   B;
        Matrix<T, N_MEAS,   N_STATES>   H;
        Matrix<T, N_STATES, N_STATES>   Q;
        Matrix<T, N_MEAS,   N_MEAS>     R;

        constexpr Result<KalmanFilter, Contracts> into_kalman_filter() && {
            auto & self = *this;
            if(not self.P.is_symmetric()) 
                return Err(Contracts::CovarianceIsNotSymmetric);

            if(not self.Q.is_symmetric())
                return Err(Contracts::ProcessNoiseIsNotSymmetric);

            if(not self.R.is_symmetric()){
                return Err(Contracts::MeasurementNoiseIsNotSymmetric);
            }

            return Ok(KalmanFilter(std::move(*this)));
        }
    };

    constexpr explicit KalmanFilter(Config && cfg):
        x_(cfg.initial_state),
        P_(cfg.initial_covariance),
        F_(cfg.F),
        B_(cfg.B),
        H_(cfg.H),
        Q_(cfg.Q),
        R_(cfg.R){;}
    
    // 预测步骤
    constexpr void predict(
        const Matrix<T, N_INPUTS, 1>& u, 
        T dt
    ) {
        // 状态预测
        x_ = state_transition_(x_, u, F_);
        
        // 协方差预测: P = F * P * Fᵀ + Q
        P_ = F_ * P_ * F_.transpose() + process_noise_(Q_, dt);
    }
    
    // 更新步骤
    constexpr void update(const Matrix<T, N_MEAS, 1>& z) {
        // 计算卡尔曼增益: K = P * Hᵀ * (H * P * Hᵀ + R)⁻¹
        const auto H_transpose = H_.transpose();
        const auto S = H_ * P_ * H_transpose + measurement_noise_(R_, z);
        const auto K = P_ * H_transpose * S.inverse();
        
        // 状态更新: x = x + K * (z - H * x)
        const auto innovation = z - observation_(x_, H_);
        x_ = x_ + K * innovation;
        
        // 协方差更新: P = (I - K * H) * P
        const auto I = Matrix<T, N_STATES, N_STATES>::identity();
        P_ = (I - K * H_) * P_;
    }
    
    // 一步预测和更新
    constexpr void step(
        const Matrix<T, N_MEAS, 1>& z,
        const Matrix<T, N_INPUTS, 1>& u,
        T dt
    ) {
        predict(u, dt);
        update(z);
    }
    
    // 获取当前状态估计
    constexpr const Matrix<T, N_STATES, 1>& state() const { return x_; }
    
    // 获取当前协方差
    constexpr const Matrix<T, N_STATES, N_STATES>& covariance() const { return P_; }
    
    // 获取卡尔曼增益（用于调试）
    constexpr Matrix<T, N_STATES, N_MEAS> compute_kalman_gain() const {
        const auto H_transpose = H_.transpose();
        const auto S = H_ * P_ * H_transpose + R_;
        return P_ * H_transpose * S.inverse();
    }
    
    // 重置滤波器
    constexpr void reset(
        const Matrix<T, N_STATES, 1>& new_state,
        const Matrix<T, N_STATES, N_STATES>& new_covariance
    ) {
        x_ = new_state;
        P_ = new_covariance;
    }

private:
    // 状态和协方差
    Matrix<T, N_STATES, 1>              x_; // 状态估计
    Matrix<T, N_STATES, N_STATES>       P_; // 估计误差协方差
    
    // 系统模型
    Matrix<T, N_STATES, N_STATES>       F_; // 状态转移矩阵
    Matrix<T, N_STATES, N_INPUTS>       B_; // 控制输入矩阵
    Matrix<T, N_MEAS, N_STATES>         H_; // 观测矩阵
    Matrix<T, N_STATES, N_STATES>       Q_; // 过程噪声协方差
    Matrix<T, N_MEAS, N_MEAS>           R_; // 测量噪声协方差
    
    // 策略对象
    [[no_unique_address]] StateTransition state_transition_ = StateTransition{};
    [[no_unique_address]] Observation observation_ = Observation{};
    [[no_unique_address]] ProcessNoise process_noise_ = ProcessNoise{};
    [[no_unique_address]] MeasurementNoise measurement_noise_ = MeasurementNoise{};
};

}
#pragma once

#include <cmath>
#include <cstdint>
namespace ymd::dsp::l1{


// 这是正确的L1自适应控制器实现
struct [[nodiscard]]L1AdaptiveController final{ 
    struct [[nodiscard]] State final{
        float state_u_prev;
        float state_x_hat_in;

        static constexpr State zero(){
            return State{0.0f, 0.0f};
        }
    };

    struct [[nodiscard]] Precomputed final{
        float kr;              // param_am / param_b
        float phi_ts;          // (1 - exp(-param_am * param_ts)) / param_am
        float exp_factor;      // exp(-param_am * param_ts)
        float k_u;             // param_ts / param_tau
        float k_u_prev;        // 1.0 - k_u
        float k_inv_muy;       // 1.0 / (phi_ts * param_b)
        float param_am;        
        float param_b;         
        float param_ts;        // 保存时间步长
    };


    struct [[nodiscard]] Config final{
        uint32_t fs;
        float param_am;
        float param_b;
        float param_tau;

        constexpr float param_ts() const {
            return 1.0f / static_cast<float>(fs);
        }

        constexpr Precomputed to_precomputed() const {
            float local_param_ts = param_ts();
            const auto k_u = local_param_ts / param_tau;
            const auto phi_ts = (1.0f - std::exp(-param_am * local_param_ts)) / param_am;
            return Precomputed{
                .kr = param_am / param_b,
                .phi_ts = phi_ts,
                .exp_factor = std::exp(-param_am * local_param_ts),
                .k_u = k_u,
                .k_u_prev = 1.0f - k_u,
                .k_inv_muy = 1.0f / (phi_ts * param_b),
                .param_am = param_am,
                .param_b = param_b,
                .param_ts = local_param_ts
            };
        }
    };

    constexpr L1AdaptiveController(const Config & cfg):
        precomputed_(cfg.to_precomputed()),
        state_(State::zero())
    {}

    float iterate(const float desired_x1, const float now_x1){
        // 计算状态误差
        float x_tilde = state_.state_x_hat_in - now_x1;

        // 估计系统不确定性
        float sigma_hat = estimator(x_tilde);

        // 控制器设计，基于L1自适应控制原理
        // 控制输入由理想控制器输出减去系统不确定性估计值
        float u = precomputed_.kr * desired_x1 - sigma_hat;
        
        // 应用滤波器：对u进行一个平滑
        float u_f = precomputed_.k_u * u + precomputed_.k_u_prev * state_.state_u_prev;
        
        // 更新上一次控制输入
        state_.state_u_prev = u_f;

        // 预测系统 - 使用欧拉积分
        float x_dot = -precomputed_.param_am * state_.state_x_hat_in + 
                      precomputed_.param_b * (u_f + sigma_hat);
        state_.state_x_hat_in += x_dot * precomputed_.param_ts;

        return u_f;
    }

private:
    // 不确定性估计器，用于估计系统的不确定性
    float estimator(float x_tilde) const {
        // 计算状态误差的加权
        float muy = precomputed_.exp_factor * x_tilde;
        // 计算不确定性估计值
        float sigma_hat = -muy * precomputed_.k_inv_muy;
        return sigma_hat;
    }

    Precomputed precomputed_;
    State state_;
};

}
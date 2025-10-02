#pragma once

#include "core/math/realmath.hpp"

namespace ymd::digipw{


struct PiController {
    struct Cofficients { 
        q24 kp;                // 比例系数
        q24 ki_discrete;       // 离散化积分系数（Ki * Ts）
        q24 max_out;          // 最大输出电压限制
        q20 err_sum_max;       // 积分项最大限制（抗饱和）
    };

    constexpr PiController(const Cofficients& cfg):
        kp_(cfg.kp),
        ki_discrete_(cfg.ki_discrete),
        max_out_(cfg.max_out),
        err_sum_max_(cfg.err_sum_max)
    {}

    constexpr void reset(){
        err_sum_ = q20(0);
    }

    constexpr auto operator()(const q24 err) {
        q24 output = CLAMP2(kp_ * err + err_sum_ * ki_discrete_, max_out_);
        err_sum_ = CLAMP(err_sum_ + err, -max_out_ - output , max_out_ - output);
        return output;
    }

private:
    q24 kp_;                // 比例系数
    q24 ki_discrete_;       // 离散化积分系数（Ki * Ts）
    q24 max_out_;          // 最大输出电压限制
    q20 err_sum_max_;       // 积分项最大限制（抗饱和）
    q24 err_sum_;           // 误差积分累加器
};


}
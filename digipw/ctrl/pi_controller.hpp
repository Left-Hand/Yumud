#pragma once

#include "core/math/realmath.hpp"

namespace ymd::digipw{


// struct PiController {
//     struct Cofficients { 
//         q20 kp;                // 比例系数
//         q20 ki_discrete;       // 离散化积分系数（Ki * Ts）
//         q20 max_out;          // 最大输出电压限制
//         q20 err_sum_max;       // 积分项最大限制（抗饱和）
//     };

//     constexpr PiController(const Cofficients& cfg):
//         kp_(cfg.kp),
//         ki_discrete_(cfg.ki_discrete),
//         max_out_(cfg.max_out),
//         err_sum_max_(cfg.err_sum_max)
//     {}

//     constexpr void reset(){
//         err_sum_ = q20(0);
//     }

//     constexpr auto operator()(const q20 err) {
//         q20 output = CLAMP2(kp_ * err + err_sum_ * ki_discrete_, max_out_);
//         err_sum_ = CLAMP(err_sum_ + err, -max_out_ - output , max_out_ - output);
//         return output;
//     }

// private:
//     q20 kp_;                // 比例系数
//     q20 ki_discrete_;       // 离散化积分系数（Ki * Ts）
//     q20 max_out_;          // 最大输出电压限制
//     q20 err_sum_max_;       // 积分项最大限制（抗饱和）
//     q20 err_sum_;           // 误差积分累加器
// };


}
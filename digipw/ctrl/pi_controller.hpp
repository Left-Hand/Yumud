#pragma once

#include "core/math/realmath.hpp"

namespace ymd::digipw{


//antiwinded pi controller
//kp + ki / s
struct [[nodiscard]] PiCofficients { 
    iq20 kp;                // 比例系数
    iq20 ki_discrete;       // 离散化积分系数（Ki * Ts）

    friend OutputStream & operator << (OutputStream & os, const PiCofficients & self){
        return os << self.kp << 
            os.splitter() << self.ki_discrete 
        ;
    }
};

#if 0
struct [[nodiscard]] PiController {

    constexpr PiController(const PiCofficients& cfg):
        kp_(cfg.kp),
        ki_discrete_(cfg.ki_discrete),
        max_out_(cfg.max_out),
        err_int_max_(MIN(static_cast<iq20>(cfg.err_int_max), std::numeric_limits<iq20>::max()))
    {}

    constexpr void reset(){
        err_int_ = 0;
    }

    constexpr auto operator()(const iq20 err) {
        // iq20 output = CLAMP2( + ki_discrete_ * iq20(err_int_), max_out_);
        const iq20 kp_contribute = kp_ * iq20(err);
        // const auto kp_contribute = kp_ * iq20(err);
        err_int_ = CLAMP(err_int_ + err * ki_discrete_, -max_out_ - kp_contribute , max_out_ - kp_contribute);
        iq20 output = CLAMP2(kp_contribute + err_int_, max_out_);
        // err_int_ = CLAMP(err_int_ + err, -err_int_max_, err_int_max_);
        return output;
    }

public:
    iq20 kp_;                // 比例系数
    iq20 ki_discrete_;       // 离散化积分系数（Ki * Ts）
    iq20 max_out_;          // 最大输出电压限制
    iq20 err_int_max_;       // 积分项最大限制（抗饱和）
    iq20 err_int_;           // 误差积分累加器
};
#endif

}
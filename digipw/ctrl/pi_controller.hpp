#pragma once

#include "core/math/realmath.hpp"

namespace ymd::digipw{


//antiwinded pi controller
//kp + ki / s

struct [[nodiscard]] PiController {
    struct [[nodiscard]] Cofficients { 
        iq20 kp;                // 比例系数
        iq20 ki_discrete;       // 离散化积分系数（Ki * Ts）
        iq20 max_out;          // 最大输出电压限制
        iq20 err_int_max;       // 积分项最大限制（抗饱和）

        constexpr PiController to_pi_controller() const noexcept {
            return PiController(*this);
        }
        friend OutputStream & operator << (OutputStream & os, const Cofficients & self){
            return os << self.kp << 
                os.splitter() << self.ki_discrete 
                << os.splitter() << self.max_out 
                << os.splitter() << self.err_int_max 
            ;
        }
    };

    constexpr PiController(const Cofficients& cfg):
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
        iq20 output = CLAMP2(kp_contribute + err_int_, max_out_);
        err_int_ = CLAMP(err_int_ + err * ki_discrete_, -max_out_ - output , max_out_ - output);
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

}
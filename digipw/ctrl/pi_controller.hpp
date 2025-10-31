#pragma once

#include "core/math/realmath.hpp"

namespace ymd::digipw{


//antiwinded pi controller
//kp + ki / s

struct [[nodiscard]] PiController {
    struct [[nodiscard]] Cofficients { 
        iq16 kp;                // 比例系数
        iq16 ki_discrete;       // 离散化积分系数（Ki * Ts）
        iq16 max_out;          // 最大输出电压限制
        iq16 err_sum_max;       // 积分项最大限制（抗饱和）

        constexpr PiController to_controller() const {
            return PiController(*this);
        }
        friend OutputStream & operator << (OutputStream & os, const Cofficients & self){
            return os << self.kp << 
                os.splitter() << self.ki_discrete 
                << os.splitter() << self.max_out 
                << os.splitter() << self.err_sum_max 
            ;
        }
    };

    using intergal_t = iq20;

    constexpr PiController(const Cofficients& cfg):
        kp_(cfg.kp),
        ki_discrete_(cfg.ki_discrete),
        max_out_(cfg.max_out),
        err_sum_max_(MIN(static_cast<intergal_t>(cfg.err_sum_max), std::numeric_limits<intergal_t>::max()))
    {}

    constexpr void reset(){
        err_sum_ = 0;
    }

    constexpr auto operator()(const iq20 err) {
        // iq16 output = CLAMP2( + ki_discrete_ * iq16(err_sum_), max_out_);
        const iq16 kp_contribute = kp_ * iq16(err);
        // const auto kp_contribute = kp_ * iq16(err);
        iq16 output = CLAMP2(kp_contribute + ki_discrete_ * iq16(err_sum_), max_out_);
        err_sum_ = CLAMP(err_sum_ + err, -err_sum_max_ - output , err_sum_max_ - output);
        // err_sum_ = CLAMP(err_sum_ + err, -err_sum_max_, err_sum_max_);
        return output;
    }

public:
    iq16 kp_;                // 比例系数
    iq16 ki_discrete_;       // 离散化积分系数（Ki * Ts）
    iq16 max_out_;          // 最大输出电压限制
    intergal_t err_sum_max_;       // 积分项最大限制（抗饱和）
    intergal_t err_sum_;           // 误差积分累加器
};

}
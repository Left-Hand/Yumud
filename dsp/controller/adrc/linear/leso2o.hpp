#pragma once

#include "../prelude.hpp"

// https://zhuanlan.zhihu.com/p/692439642

// function [dx1,dx2] = fcn(y,u,x1,x2)
// % X=[电机转速rad/s, 总扰动]
// % 系统开环增益
// b0=0.8/6.26822e-4;
// % LESO带宽 rad/s
// w=100;
// % 观测器增益
// g1=2*w;
// g2=w*w；
// % 状态观测器
// dx1=x2+b0*u+g1*(y-x1);
// dx2=g2*(y-x1);

// https://blog.csdn.net/chase2025/article/details/146990194

namespace ymd::dsp::adrc{


//线性拓展状态观测器
template<>
struct [[nodiscard]] LinearExtendedStateObserver<iq20, 2>{
public:
    using Self = LinearExtendedStateObserver;
    using State = StateVector<iq20, 2>;

    struct [[nodiscard]] Config{
        uint32_t fs;
        iq8 b0;
        iq16 w;
    };

    constexpr explicit LinearExtendedStateObserver(const Config & cfg){reconf(cfg);}

    constexpr void reconf(const Config & cfg){
        b0_ = cfg.b0;
        dt_ = uq32::from_rcp(cfg.fs);
        g1_ = 2 * cfg.w;
        g2_ = cfg.w * cfg.w;
    }

    constexpr State update(const State state,  const iq16 y, const iq16 u) const {
        auto & self = *this;
        return {
            state[0] + (state[1] + self.b0_ * u + self.g1_ * (y - state[0])) * self.dt_,
            state[1] + self.g2_ * (y - state[0]) * self.dt_
        };
    }
private:

    iq8 b0_;
    uq32 dt_;
    iq20 g1_;
    iq20 g2_;


};

}
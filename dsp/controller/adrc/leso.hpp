#pragma once

#include "utils.hpp"

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


namespace ymd::dsp{

class Leso{
public:
    struct Config{
        iq8 b0;
        iq16 w;
        uint32_t fs;
    };

    constexpr explicit Leso(const Config & cfg){reconf(cfg);}

    constexpr void reconf(const Config & cfg){
        b0_ = cfg.b0;
        delta_ = 1_iq20 / cfg.fs;
        g1_ = 2 * cfg.w;
        g2_ = cfg.w * cfg.w;
    }

    constexpr void update(const iq16 y, const iq16 u){
        this->state_ = forward(*this, y, u, this->state_);
    }

    constexpr void reset(){
        this->state_ = State{0, 0};
    }

    constexpr const auto & state() const {return this->state_;}
    constexpr auto disturbance() const {return this->state_[1];}
private:
    using Self = Leso;
    using State = StateVector<iq20, 2>;

    State state_;
    iq8 b0_;
    iq20 delta_;
    iq20 g1_;
    iq20 g2_;

    static constexpr State forward(const Self & self, const iq16 y, const iq16 u, const State x){
        return {
            x[0] + (x[1] + self.b0_ * u + self.g1_ * (y - x[0])) * self.delta_,
            x[1] + self.g2_ * (y - x[0]) * self.delta_
        };
    }
};

}
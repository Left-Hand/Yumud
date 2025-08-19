#pragma once

#include "utils.hpp"

// https://zhuanlan.zhihu.com/p/692439642


namespace ymd::dsp{

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


class Leso{
public:
    struct Config{
        q8 b0;
        q16 w;
        uint32_t fs;
    };

    constexpr Leso(const Config & cfg){reconf(cfg);}

    constexpr void reconf(const Config & cfg){
        b0_ = cfg.b0;
        delta_ = 1_q20 / cfg.fs;
        g1_ = 2 * cfg.w;
        g2_ = cfg.w * cfg.w;
    }

    constexpr void update(const q16 y, const q16 u){
        this->state_ = forward(*this, y, u, this->state_);
    }

    constexpr void reset(){
        this->state_ = State{0, 0};
    }

    constexpr const auto & get() const {return this->state_;}
    constexpr const auto & get_disturbance() const {return this->state_[1];}
private:
    using Self = Leso;
    using State = StateVector<q20, 2>;

    State state_;
    q8 b0_;
    q20 delta_;
    q20 g1_;
    q20 g2_;

    static constexpr State forward(const Self & self, const q16 y, const q16 u, const State x){
        return {
            x[0] + (x[1] + self.b0_ * u + self.g1_ * (y - x[0])) * self.delta_,
            x[1] + self.g2_ * (y - x[0]) * self.delta_
        };
    }
};

}
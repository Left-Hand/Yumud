#pragma once

#include "core/math/iq/fixed_t.hpp"

namespace ymd::dsp{


class SecondOrderTrackingDifferentiator{
public:
    struct Config{
        uint32_t fs;
        iq8 r;
    };

    struct State{
        iq21 position;
        iq22 speed;

        constexpr void reset(){
            position = 0;
            speed = 0;
        }
    };


    constexpr explicit SecondOrderTrackingDifferentiator(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        r_ = cfg.r;
        dt_ = 1_iq24 / cfg.fs;
    }

    constexpr void reset(){
        state_.reset();
    }

    constexpr void update(const iq16 u){
        const iq8 r = r_;
        const iq8 r_2 = r * r;

        const auto x1 = state_.position;
        const auto x2 = state_.speed;

        state_.position += dt_ * x2; 

        //临界二阶阻尼系统
        // G(s) = 1 / (s ^ 2 + 2 * r * s + r ^ 2)
        state_.speed += dt_ * (iq16(- 2 * x2) * r + (iq16(u - x1) * r_2));
    }

    constexpr const State & state() const {return state_;}
private:
    iq8 r_ = 0;
    iq24 dt_ = 0;

    State state_;
};

}
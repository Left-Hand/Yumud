#pragma once

// #include "dsp/filter/butterworth/ButterSideFilter.hpp"
// #include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"

#include "core/math/real.hpp"
#include "types/vector2/vector2.hpp"

namespace nudec::_2023E{

//跟踪微分器 用于平滑输入
class TdVec2{
public:
    struct Config{
        q8 r;
        q20 max_spd;
        uint fs;
    };

    using E = q20;
    using T = Vector2_t<q20>;

    using State = std::array<T, 2>;
    using Self = TdVec2;
    
    TdVec2(const Config & cfg){
        reset();
        reconf(cfg);
    }

    //impure fn
    constexpr void update(const T targ){
        state_ = forward(*this, state_, targ);
    }

    //impure fn
    constexpr void reset(){
        // pass
    }

    //impure fn
    constexpr void reconf(const Config & cfg){
        r_ = cfg.r;
        dt_ = 1_q20 / cfg.fs;
        max_spd_ = cfg.max_spd;
    }
    
    //impure fn
    constexpr const auto & get() const {
        return state_;
    }
private:
    q8 r_;
    q20 dt_;
    q20 max_spd_;
    State state_ {};

    //pure fn
    static constexpr State forward(const Self & self, const State state, const T u){
        const auto r = self.r_;
        const auto r_2 = r * r;
        // const auto r_3 = r_2 * r;
        const auto dt = self.dt_;
        const auto max_spd = self.max_spd_;

        const auto x1 = state[0];
        const auto x2 = state[1];

        return {
            state[0] + x2 * dt, 
            (state[1] + (- 2 * r * x2 - r_2 * (x1 - u)) * dt).clampmax(max_spd)
        };
    }

    
};

}
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
        q12 kp;
        q12 kd;
        q16 max_spd;
        q16 max_acc;
        uint fs;
    };

    using E = q16;
    using T = Vector2_t<q16>;

    using State = std::array<T, 3>;
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
        kp_ = cfg.kp;
        kd_ = cfg.kd;
        dt_ = 1_q16 / cfg.fs;
        max_spd_ = cfg.max_spd;
        max_acc_ = cfg.max_acc;
    }
    
    //impure fn
    [[nodiscard]]
    constexpr const auto & get() const {
        return state_;
    }
private:
    q12 kp_;
    q12 kd_;
    q16 dt_;
    q16 max_spd_;
    q16 max_acc_;
    State state_ {};

    //pure fn
    [[nodiscard]]
    static constexpr State forward(const Self & self, const State state, const T u){
        // const auto r_3 = r_2 * r;
        const auto dt = self.dt_;
        // const auto max_spd = self.max_spd_;
        // const auto max_acc = self.max_acc_;

        const auto pos = state[0];
        const auto spd = state[1];
        const auto acc = state[2];

        const auto raw_a = ((q12(self.kp_) * (Vector2_t<q12>(u - pos)))
             - (self.kd_ * spd));
        // DEBUG_PRINTLN(raw_a, self.max_acc_);
        return {
            pos + spd * dt, 
            (spd + acc * dt)
            .clampmax(self.max_spd_)
            ,
            
            // raw_a
            {CLAMP2(raw_a.x, self.max_acc_), CLAMP2(raw_a.y, self.max_acc_)}
            // .clampmax(self.max_acc_)
            // .clampmax(10.0_r)
            // (spd + acc * dt).clampmax(max_spd),
            // (acc + (-self.kd * spd, -self.kp_ * (pos - u)))
            // (state[1] + (- 2 * r * spd - r_2 * (pos - u)) * dt)
        };


    }

    
};

}
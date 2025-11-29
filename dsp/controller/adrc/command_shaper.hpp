#pragma once

#include "utils.hpp"
#include "types/vectors/vector2.hpp"

namespace ymd::dsp{

//跟踪微分器 用于平滑输入
class TdVec2{
public:
    struct Config{
        iq12 kp;
        iq12 kd;
        iq16 max_spd;
        iq16 max_acc;
        uint32_t fs;
    };

    using E = iq16;
    using T = Vec2<iq16>;

    using State = std::array<T, 3>;
    using Self = TdVec2;
    
    TdVec2(const Config & cfg){
        reset();
        reconf(cfg);
    }

    //impure fn
    void update(const T targ){
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
        dt_ = 1_iq16 / cfg.fs;
        max_spd_ = cfg.max_spd;
        max_acc_ = cfg.max_acc;
    }
    
    //impure fn
    [[nodiscard]]
    constexpr const auto & state() const {
        return state_;
    }
private:
    iq12 kp_;
    iq12 kd_;
    iq16 dt_;
    iq16 max_spd_;
    iq16 max_acc_;
    State state_ {T::ZERO, T::ZERO, T::ZERO};

    //pure fn
    [[nodiscard]]
    // static constexpr State 
    static State 
    forward(const Self & self, const State state, const T u){
        // const auto r_3 = r_2 * r;
        const auto dt = self.dt_;
        // const auto max_spd = self.max_spd_;
        // const auto max_acc = self.max_acc_;

        const auto pos = state[0];
        const auto spd = state[1];
        // const auto acc = state[2];

        // const auto raw_a = ((iq12(self.kp_) * (Vec2<iq12>(u - pos)))
        //      - (self.kd_ * spd));
        // DEBUG_PRINTLN(raw_a, self.max_acc_);

        const auto pos_err = Vec2<iq12>(u - pos);
        const auto dist = pos_err.length();
        const auto norm_pos_err = pos_err / dist;
        const auto expect_spd = std::sqrt(2 * self.max_acc_ * dist);

        // DEBUG_PRINTLN(spd.normalized() * expect_spd);
        return {
            pos + spd * dt, 
            spd.move_toward(norm_pos_err * expect_spd, self.max_acc_ * self.dt_)
            .clampmax(self.max_spd_)
            ,
            norm_pos_err * expect_spd
            // raw_a
            // {CLAMP2(raw_a.x, self.max_acc_), CLAMP2(raw_a.y, self.max_acc_)}
            // .clampmax(self.max_acc_)
            // .clampmax(10.0_r)
            // (spd + acc * dt).clampmax(max_spd),
            // (acc + (-self.kd * spd, -self.kp_ * (pos - u)))
            // (state[1] + (- 2 * r * spd - r_2 * (pos - u)) * dt)
        };


    }

    
};



}
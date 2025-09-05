#pragma once

#include "utils.hpp"
#include "types/vectors/vector2.hpp"
#include "tracking_differentiator.hpp"

namespace std{
    template<size_t Q, size_t Q2>
    [[nodiscard]] __fast_inline constexpr iq_t<Q> copysign(const iq_t<Q> x, const iq_t<Q2> s){
        // return iq_t<Q>(_iq<Q>::from_i32((
        //     (std::bit_cast<int32_t>(x.value) & 0x7fff'ffff) | 
        //     (std::bit_cast<int32_t>(s.value) & 0x8000'0000)
        // )));

        return s > 0 ? x : -x;
        // return SIGN_AS(x,s);
    }
}
namespace ymd::dsp{

//跟踪微分器 用于平滑输入
class TdVec2{
public:
    struct Config{
        q12 kp;
        q12 kd;
        q16 max_spd;
        q16 max_acc;
        uint32_t fs;
    };

    using E = q16;
    using T = Vec2<q16>;

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

        // const auto raw_a = ((q12(self.kp_) * (Vec2<q12>(u - pos)))
        //      - (self.kd_ * spd));
        // DEBUG_PRINTLN(raw_a, self.max_acc_);

        const auto pos_err = Vec2<q12>(u - pos);
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


//跟踪微分器 用于平滑输入

class CommandShaper1{
public:
    using Self = CommandShaper1;

    struct Config{
        q12 kp;
        q12 kd;
        q16 max_spd;
        q16 max_acc;
        uint32_t fs;
    };

    using E = q16;
    using T = q16;

    using State = std::array<q20, 2>;

    
    CommandShaper1(const Config & cfg){
        reset();
        reconf(cfg);
    }

    //impure fn
    void update(const T targ){
        // __nopn(1);
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
    constexpr const State get() const {
        return {
            state_[0],
            state_[1] * 0.6_r
            // state_[1]
        };
    }
// private:
public:
    q20 kp_;
    q20 kd_;
    q20 dt_;
    q20 max_spd_;
    q20 max_acc_;
    State state_ {};

    dsp::TrackingDifferentiatorByOrders<2> lpf = {dsp::TrackingDifferentiatorByOrders<2>::Config{
        .r = 80.0_r,
        .fs = 1000
    }};

    //pure fn
    [[nodiscard]]
    static constexpr __fast_inline State 
    // static State 
    forward(Self & self, const State & state, const T u0){
        // const auto r_3 = r_2 * r;
        const auto dt = self.dt_;
        // const auto max_spd = self.max_spd_;
        // const auto max_acc = self.max_acc_;

        const auto pos = state[0];
        const auto spd = state[1];
        // const auto acc = state[2];

        // const auto raw_a = ((q12(self.kp_) * (<q12>(u0 - pos)))
        //      - (self.kd_ * spd));
        // DEBUG_PRINTLN(raw_a, self.max_acc_);

        // static dsp::LowpassFilter<real_t> lpf = {dsp::LowpassFilter<real_t>::Config{
        //     .fc = 300,
        //     .fs = 1000
        // }};

        // static 

        self.lpf.update(u0);
        const auto u = self.lpf.get()[0];
        // const auto u = u0;
        const auto pos_err = u - pos;
        const auto dist = ABS(pos_err);
        // const auto expect_spd = CLAMP2(SIGN_AS(std::sqrt(2.0_r * self.max_acc_ * dist), pos_err), self.max_spd_);
        // const auto expect_spd = iq_t<16>(CLAMP2(self.lpf.get()[1] + SIGN_AS(std::sqrt(2.0_r * self.max_acc_ * dist), pos_err), self.max_spd_));
        // DEBUG_PRINTLN(u0, u, pos, spd, expect_spd, self.max_spd_, self.lpf.get());
        
        // DEBUG_PRINTLN(expect_spd);
        // if(dist > 0.1_r){
        if(true){
            auto expect_spd = std::copysign(std::sqrt(2.0_r * self.max_acc_ * dist), pos_err);
            // auto expect_spd = std::copysign(std::sqrt(1.57_r * self.max_acc_ * dist), pos_err);
            if(spd * self.lpf.get()[1] < 0) expect_spd += self.lpf.get()[1];
            const auto spd_cmd = q20(CLAMP2(expect_spd, self.max_spd_));
            return {
                pos + spd * dt, 
                STEP_TO(spd, spd_cmd, q20((self.max_acc_)* self.dt_)),
            };
        }else{
            return {
                pos + spd * dt, 
                // STEP_TO(spd, q16(0), q16()),
                CLAMP2(spd + CLAMP2( -self.kd_ * spd + self.kp_ * pos_err, self.max_acc_) * dt, self.max_spd_),
            };
        }

    }

    
};

}
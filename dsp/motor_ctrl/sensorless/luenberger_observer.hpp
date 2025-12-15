#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

#include "digipw/prelude/abdq.hpp"

// 全面推导龙伯格观测器相关公式
// https://zhuanlan.zhihu.com/p/960435762

namespace ymd::dsp::motor_ctl {

class LuenbergerObserver{
public:

    struct Config{
        uint32_t fs;
        // iq20 R = 10.0;
        // iq20 L = 2.64E-3;
        // int32_t l1 = -22000;
        // int32_t l2 = 64000;
        iq20 phase_inductance;
        iq20 phase_resistance;
    };

    struct State{
        digipw::AlphaBetaCoord<iq20> i;
        digipw::AlphaBetaCoord<iq20> e;

        constexpr void reset(){
            i = digipw::AlphaBetaCoord<iq20>(0, 0);
            e = digipw::AlphaBetaCoord<iq20>(0, 0);
        }

        constexpr Angular<iq20> angle() const {
            return e.angle();
        }
    };

    constexpr explicit LuenbergerObserver(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        //exp 43
        auto & self = *this;
        const auto R = cfg.phase_resistance;
        const auto L = cfg.phase_inductance;
        const auto dt = iq20(1) / cfg.fs;

        self.Tr_L = iq20(- dt * R / L);
        self.T_L  = iq20(dt / L);
        self.l1T  = iq20(dt * R / L - 2);
        self.l2T  = iq20(L * cfg.fs);
    }

    constexpr void reset(){
        state_.reset();
    }


    constexpr void update(const State & meas){
        state_ = iterator_state(meas);
    }

    constexpr const State & state() const {
        return state_;
    }

public:
    State state_;
    iq20 turns_ = 0;

    iq20 Tr_L = 0;
    iq20 T_L  = 0;
    iq20 l1T  = 0;
    iq20 l2T  = 0;

private:
    constexpr State iterator_state(const State & meas) const {
        const auto err_i = state_.i - meas.i;
        const auto next_i = (1 + Tr_L) * state_.i + T_L * (meas.e - state_.e) + l1T * err_i; 
        const auto next_e = state_.e + l2T * err_i;
        return State{next_i, next_e};
    }
};


// class LesObserver{
// public:
//     LesObserver(const uint fc, const iq20 b0, const iq20 wc):
//         fc_(fc), b0_(b0), wo_(3 * wc){;}

//     void update(const iq20 w_meas, const iq20 qcurr_ref){
//         const iq20 err = w_hat_ - w_meas;

//         const iq20 last_f_ = f_;

//         w_hat_ += last_f_ - 2 * wo_ * err + b0_ * qcurr_ref / fc_;
//         f_ += - (wo_ * wo_) * err / fc_;
//     }
// private:
//     uint fc_;
//     iq20 b0_;
//     iq20 wo_;

//     iq20 w_hat_ = 0;
//     iq20 f_ = 0;
// };




}
#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

// 全面推导龙伯格观测器相关公式
// https://zhuanlan.zhihu.com/p/960435762

namespace ymd::dsp::motor_ctl {

class LuenbergerObserver{
public:

    struct Config{
        uint32_t fs;
        // q20 R = 10.0;
        // q20 L = 2.64E-3;
        // int32_t l1 = -22000;
        // int32_t l2 = 64000;
        q20 phase_inductance;
        q20 phase_resistance;
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
        const auto dt = q20(1) / cfg.fs;

        self.Tr_L = q20(- dt * R / L);
        self.T_L  = q20(dt / L);
        self.l1T  = q20(dt * R / L - 2);
        self.l2T  = q20(L * cfg.fs);
    }

    constexpr void reset(){
        i_alpha_ = 0;
        i_beta_ = 0;
        e_alpha_ = 0;
        e_beta_ = 0;
    }

    constexpr void update(auto alphabeta_volt, auto alphabeta_curr) {
        const auto [Valpha, Vbeta] = alphabeta_volt;
        const auto [Ialpha, Ibeta] = alphabeta_curr;
        recalc(i_alpha_, e_alpha_, Valpha, Ialpha);
        recalc(i_beta_, e_beta_, Vbeta, Ibeta);

        turns_ = frac(atan2pu<16>(-e_alpha_, e_beta_));
        // turns_ = frac(atan2pu(-Ealpha, Ebeta));
    }

    constexpr Angle<iq16> angle() const {
        return Angle<iq16>::from_turns(q16(turns_));
    }

public:

    q20 i_alpha_ = 0;
    q20 i_beta_ = 0;
    q20 e_alpha_ = 0;
    q20 e_beta_ = 0;
    q20 turns_ = 0;
    q20 Tr_L = 0;
    q20 T_L  = 0;
    q20 l1T  = 0;
    q20 l2T  = 0;

private:
    __fast_inline
    constexpr void recalc(q20 __restrict & i, q20 __restrict & e, const q20 meas_volt, const q20 meas_i){
        const auto err_i = (i - meas_i);
        i += Tr_L * i + T_L * (meas_volt - e) + l1T * err_i; 
        e += l2T * err_i;
    };

};


// class LesObserver{
// public:
//     LesObserver(const uint fc, const q20 b0, const q20 wc):
//         fc_(fc), b0_(b0), wo_(3 * wc){;}

//     void update(const q20 w_meas, const q20 qcurr_ref){
//         const q20 err = w_hat_ - w_meas;

//         const q20 last_f_ = f_;

//         w_hat_ += last_f_ - 2 * wo_ * err + b0_ * qcurr_ref / fc_;
//         f_ += - (wo_ * wo_) * err / fc_;
//     }
// private:
//     uint fc_;
//     q20 b0_;
//     q20 wo_;

//     q20 w_hat_ = 0;
//     q20 f_ = 0;
// };




}
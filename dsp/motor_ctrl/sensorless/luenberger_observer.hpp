#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

namespace ymd::dsp::motor_ctl {

class LuenbergerObserver{
public:

    struct Config{

    };

    constexpr LuenbergerObserver(const Config & cfg){
        reset();
    }

    constexpr void reset(){
        i_alpha_ = 0;
        i_beta_ = 0;
        e_alpha_ = 0;
        e_beta_ = 0;
    }
    


    static constexpr double freq = 32000;
    static constexpr double T = 1.0 / freq;



    // static constexpr double R = 0.9;
    // static constexpr double L = 0.16E-3;

    // static constexpr double R = 2.7;
    // static constexpr double L = 0.72E-3;
    // static constexpr double l1 = -20000;
    // static constexpr double l2 = 52000;

    static constexpr double R = 10.0;
    static constexpr double L = 2.64E-3;
    static constexpr double l1 = -22000;
    static constexpr double l2 = 64000;
    // static constexpr double l1 = -18000;
    // static constexpr double l2 = 102000;

    static constexpr auto Tr_L = q20(- T * R / L);
    static constexpr auto T_L  = q20(T / L);
    static constexpr auto l1T  = q20(l1 * T);
    static constexpr auto l2T  = q20(l2 * T);



    constexpr void update(auto alpha_beta_volt, auto alpha_beta_curr) {
        const auto [Valpha, Vbeta] = alpha_beta_volt;
        const auto [Ialpha, Ibeta] = alpha_beta_curr;
        recalc(i_alpha_, e_alpha_, Valpha, Ialpha);
        recalc(i_beta_, e_beta_, Vbeta, Ibeta);

        turns_ = frac(atan2pu<16>(e_beta_, e_alpha_));
    }

    constexpr Angle<q16> angle() const {
        return Angle<q16>::from_turns(q16(turns_));
    }

public:

    q20 i_alpha_ = 0;
    q20 i_beta_ = 0;
    q20 e_alpha_ = 0;
    q20 e_beta_ = 0;
    q20 turns_ = 0;

private:
    __fast_inline
    static constexpr void recalc(q20 __restrict & i, q20 __restrict & e, const q20 meas_volt, const q20 meas_i){
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
#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

namespace ymd::motor_ctl {

class LuenbergerObserver{
public:
    // LuenbergerObserver(const q16 _h, const q16 _Ls, const q16 _Rs, const q16 _Tc, const q16 _omega);
    constexpr LuenbergerObserver(){
        reset();
    }

    constexpr void reset(){
        i_alpha_ = 0;
        i_beta_ = 0;
        e_alpha_ = 0;
        e_beta_ = 0;
    }
    


    static constexpr double freq = 25000;
    static constexpr double T = 1.0 / freq;
    static constexpr double R = 0.2;
    static constexpr double L = 0.14E-3;

    static constexpr double l1 = -22000;
    static constexpr double l2 = 82000;
    // static constexpr double l1 = -10;
    // static constexpr double l2 =100;

    static constexpr auto Tr_L = q20(- T * R / L);
    static constexpr auto T_L  = q20(T / L);
    static constexpr auto l1T  = q20(l1 * T);
    static constexpr auto l2T  = q20(l2 * T);


    constexpr void update(q20 Valpha, q20 Vbeta, q20 Ialpha, q20 Ibeta) {


        recalc(i_alpha_, e_alpha_, Valpha, Ialpha);
        recalc(i_beta_, e_beta_, Vbeta, Ibeta);

        theta_ = atan2<20>(e_beta_, e_alpha_);
    }

    constexpr q20 theta() const {
        return theta_;
    }

public:

    q20 i_alpha_ = 0;
    q20 i_beta_ = 0;
    q20 e_alpha_ = 0;
    q20 e_beta_ = 0;
    q20 theta_ = 0;

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
//     LesObserver(const uint fc, const q16 b0, const q16 wc):
//         fc_(fc), b0_(b0), wo_(3 * wc){;}

//     void update(const q16 w_meas, const q16 qcurr_ref){
//         const q16 err = w_hat_ - w_meas;

//         const q16 last_f_ = f_;

//         w_hat_ += last_f_ - 2 * wo_ * err + b0_ * qcurr_ref / fc_;
//         f_ += - (wo_ * wo_) * err / fc_;
//     }
// private:
//     uint fc_;
//     q16 b0_;
//     q16 wo_;

//     q16 w_hat_ = 0;
//     q16 f_ = 0;
// };




}
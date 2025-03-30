#include "RolbgObserver.hpp"
#include "core/math/realmath.hpp"

// https://zhuanlan.zhihu.com/p/628987371

namespace ymd::foc{
// RolbgObserver::RolbgObserver(const q16 _h, const q16 _Ls, const q16 _Rs, const q16 _Tc, const q16 _omega)
//     : h(_h), Ls(_Ls), Rs(_Rs), Tc(_Tc), omega(_omega){}


scexpr double freq = 25000;
scexpr double T = 1.0 / freq;
scexpr double R = 1.2;
scexpr double L = 1.25E-3;

scexpr double l1 = -20000;
scexpr double l2 = 210000;
// scexpr double l1 = -10;
// scexpr double l2 =100;

scexpr auto Tr_L = q20(- T * R / L);
scexpr auto T_L  = q20(T / L);
scexpr auto l1T  = q20(l1 * T);
scexpr auto l2T  = q20(l2 * T);


void RolbgObserver::reset() {
    i_alpha_ = 0;
    i_beta_ = 0;
    e_alpha_ = 0;
    e_beta_ = 0;
}

__fast_inline
static constexpr void recalc(q20 __restrict & i, q20 __restrict & e, const q20 meas_volt, const q20 meas_i){
    const auto err_i = (i - meas_i);
    i += Tr_L * i + T_L * (meas_volt - e) + l1T * err_i; 
    e += l2T * err_i;
};

void RolbgObserver::update(q20 Valpha, q20 Vbeta, q20 Ialpha, q20 Ibeta) {


    recalc(i_alpha_, e_alpha_, Valpha, Ialpha);
    recalc(i_beta_, e_beta_, Vbeta, Ibeta);

    theta_ = atan2<20>(e_beta_, e_alpha_);
}

}

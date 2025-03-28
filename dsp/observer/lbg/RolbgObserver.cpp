#include "RolbgObserver.hpp"
#include "core/math/realmath.hpp"

namespace ymd::foc{
// RolbgObserver::RolbgObserver(const q16 _h, const q16 _Ls, const q16 _Rs, const q16 _Tc, const q16 _omega)
//     : h(_h), Ls(_Ls), Rs(_Rs), Tc(_Tc), omega(_omega){}


void RolbgObserver::init() {
    reset();
}

void RolbgObserver::reset() {
    i_alpha_ = 0;
    i_beta_ = 0;
    e_alpha_ = 0;
    e_beta_ = 0;
}



void RolbgObserver::update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta) {


    scexpr double freq = 25000;
    scexpr double T = 1.0 / freq;
    scexpr double R = 1.8;
    scexpr double L = 1.45E-3;

    scexpr double l1 = -20000;
    scexpr double l2 = 210000;

    scexpr q16 _1_l1T_Tr_L = q16(1 + l1 * T - T * R / L);
    scexpr q16 T_L = q16(T / L);
    scexpr q16 l1T = q16(l1 * T);
    scexpr q16 l2T = q16(l2 * T);


    auto update = [](q16 & i, q16 & e, const q16 est_i, const q16 est_volt, const q16 meas_volt, const q16 meas_i){
        i = _1_l1T_Tr_L * i - T_L * est_volt + T_L * meas_volt - l1T * meas_i;
        e = est_volt + l2T * (est_i - meas_i);
    };

    update(i_alpha_, e_alpha_, i_alpha_, e_alpha_, Valpha, Ialpha);
    update(i_beta_, e_beta_, i_beta_, e_beta_, Vbeta, Ibeta);

    theta_ = atan2(e_beta_, e_alpha_);
}

}

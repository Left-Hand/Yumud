#include "RolbgObserver.hpp"

namespace ymd::foc{
// RolbgObserver::RolbgObserver(const iq_t _h, const iq_t _Ls, const iq_t _Rs, const iq_t _Tc, const iq_t _omega)
//     : h(_h), Ls(_Ls), Rs(_Rs), Tc(_Tc), omega(_omega){}


void RolbgObserver::init() {
    reset();
}

void RolbgObserver::reset() {
    _i_alpha = 0;
    _i_beta = 0;
    _e_alpha = 0;
    _e_beta = 0;
}



void RolbgObserver::update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta) {


    scexpr double freq = 50000;
    scexpr double T = 1.0 / freq;
    scexpr double R = 7.1;
    scexpr double L = 1.45E-3;

    scexpr double l1 = -20000;
    scexpr double l2 = 210000;

    // scexpr double l1 = -6000;
    // scexpr double l2 = 70000;

    scexpr iq_t _1_l1T_Tr_L = iq_t(1 + l1 * T - T * R / L);
    scexpr iq_t T_L = iq_t(T / L);
    scexpr iq_t l1T = iq_t(l1 * T);
    scexpr iq_t l2T = iq_t(l2 * T);


    auto update = [](iq_t & i, iq_t & e, const iq_t est_i, const iq_t est_volt, const iq_t meas_volt, const iq_t meas_i){
        i = _1_l1T_Tr_L * i - T_L * est_volt + T_L * meas_volt - l1T * meas_i;
        e = est_volt + l2T * (est_i - meas_i);
    };

    update(_i_alpha, _e_alpha, _i_alpha, _e_alpha, Valpha, Ialpha);
    update(_i_beta, _e_beta, _i_beta, _e_beta, Vbeta, Ibeta);

    _theta = atan2(-_e_alpha, _e_beta);
}

}

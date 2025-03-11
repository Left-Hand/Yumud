#include "RolbgObserver.hpp"

namespace ymd::foc{
// RolbgObserver::RolbgObserver(const iq_t<16> _h, const iq_t<16> _Ls, const iq_t<16> _Rs, const iq_t<16> _Tc, const iq_t<16> _omega)
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



void RolbgObserver::update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta) {


    scexpr double freq = 50000;
    scexpr double T = 1.0 / freq;
    scexpr double R = 7.1;
    scexpr double L = 1.45E-3;

    scexpr double l1 = -20000;
    scexpr double l2 = 210000;

    // scexpr double l1 = -6000;
    // scexpr double l2 = 70000;

    scexpr iq_t<16> _1_l1T_Tr_L = iq_t<16>(1 + l1 * T - T * R / L);
    scexpr iq_t<16> T_L = iq_t<16>(T / L);
    scexpr iq_t<16> l1T = iq_t<16>(l1 * T);
    scexpr iq_t<16> l2T = iq_t<16>(l2 * T);


    auto update = [](iq_t<16> & i, iq_t<16> & e, const iq_t<16> est_i, const iq_t<16> est_volt, const iq_t<16> meas_volt, const iq_t<16> meas_i){
        i = _1_l1T_Tr_L * i - T_L * est_volt + T_L * meas_volt - l1T * meas_i;
        e = est_volt + l2T * (est_i - meas_i);
    };

    update(_i_alpha, _e_alpha, _i_alpha, _e_alpha, Valpha, Ialpha);
    update(_i_beta, _e_beta, _i_beta, _e_beta, Vbeta, Ibeta);

    // _theta = atan2(-_e_alpha, _e_beta);
    // _theta = atan2(_e_beta, _e_alpha) + real_t(PI/2);
    _theta = atan2(_e_beta, _e_alpha);
}

}

#include "NonlinearObserver.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::foc;

// scexpr __fast_inline q16 wrap_pm_pi(const q16 x){
//     return fposmodp(x + q16(PI), q16(TAU)) - q16(PI);
// }

scexpr __fast_inline q16 sq(const q16 x){
    return x * x;
}

void NonlinearObserver::reset(){
    V_alpha_beta_last_[0] = 0;
    V_alpha_beta_last_[1] = 0;
    flux_state_mf_[0] = 0;
    flux_state_mf_[1] = 0;
}


void NonlinearObserver::update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta){
    // Algorithm based on paper: Sensorless Control of Surface-Mount Permanent-Magnet Synchronous Motors Based on a Nonlinear Observer
    // http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
    // In particular, equation 8 (and by extension eqn 4 and 6).

    // The V_alpha_beta applied immedietly prior to the current measurement associated with this cycle
    // is the one computed two cycles ago. To get the correct measurement, it was stored twice:
    // once by final_v_alpha/final_v_beta in the current control reporting, and once by V_alpha_beta_memory.

    const q16 I_alpha_beta[2] = {Ialpha, Ibeta};
    // alpha-beta vector operations
    q16 eta_mf[2];
    for (size_t i = 0; i < 2; ++i) {
        // flux dynamics (prediction)
        q16 x_dot = -config_.phase_resistance * I_alpha_beta[i] + V_alpha_beta_last_[i];
        // integrate prediction to current timestep
        flux_state_mf_[i] += x_dot;

        // eta is the estimated permanent magnet flux (see paper eqn 6)
        eta_mf[i] = flux_state_mf_[i] - (config_.phase_inductance * config_.freq) * I_alpha_beta[i];
    }

    // Non-linear observer (see paper eqn 8):
    q16 pm_flux_sqr_mf_2 = sq(config_.pm_flux_linkage * config_.freq);
    q16 est_pm_flux_sqr_mf_2 = sq(eta_mf[0]) + sq(eta_mf[1]);
    q16 eta_factor = 0.5_r * (config_.observer_gain / pm_flux_sqr_mf_2) * (pm_flux_sqr_mf_2 - est_pm_flux_sqr_mf_2);



    // alpha-beta vector operations
    for (size_t i = 0; i < 2; ++i) {
        // add observer action to flux estimate dynamics
        q16 x_dot = eta_factor * eta_mf[i];
        // convert action to discrete-time
        flux_state_mf_[i] += x_dot;
        // update new eta
        eta_mf[i] = flux_state_mf_[i] - (config_.phase_inductance * config_.freq) * I_alpha_beta[i];
    }

    // Flux state estimation done, store V_alpha_beta for next timestep
    V_alpha_beta_last_[0] = Valpha;
    V_alpha_beta_last_[1] = Vbeta;

    // phase_ = atan2(eta_mf[1], eta_mf[0]);
    phase_ = atan2(eta_mf[1], eta_mf[0]);
}
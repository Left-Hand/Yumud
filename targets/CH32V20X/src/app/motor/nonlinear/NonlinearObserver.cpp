#include "NonlinearObserver.hpp"

using namespace ymd::foc;

scexpr __fast_inline iq_t wrap_pm_pi(const iq_t x){
    return fposmodp(x + iq_t(PI), iq_t(TAU)) - iq_t(PI);
}

scexpr __fast_inline iq_t sq(const iq_t x){
    return x * x;
}

void NonlinearObserver::reset(){
    V_alpha_beta_last_[0] = 0;
    V_alpha_beta_last_[1] = 0;
    flux_state_mf_[0] = 0;
    flux_state_mf_[1] = 0;
}

void NonlinearObserver::init(){
    reset();
}   

void NonlinearObserver::update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta){
    // Algorithm based on paper: Sensorless Control of Surface-Mount Permanent-Magnet Synchronous Motors Based on a Nonlinear Observer
    // http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
    // In particular, equation 8 (and by extension eqn 4 and 6).

    // The V_alpha_beta applied immedietly prior to the current measurement associated with this cycle
    // is the one computed two cycles ago. To get the correct measurement, it was stored twice:
    // once by final_v_alpha/final_v_beta in the current control reporting, and once by V_alpha_beta_memory.

    iq_t I_alpha_beta[2] = {Ialpha, Ibeta};
    // alpha-beta vector operations
    iq_t eta_mf_[2];
    for (size_t i = 0; i < 2; ++i) {
        // flux dynamics (prediction)
        iq_t x_dot = -config_.phase_resistance * I_alpha_beta[i] + V_alpha_beta_last_[i];
        // integrate prediction to current timestep
        flux_state_mf_[i] += x_dot;

        // eta is the estimated permanent magnet flux (see paper eqn 6)
        eta_mf_[i] = flux_state_mf_[i] - (config_.phase_inductance * config_.freq) * I_alpha_beta[i];
    }

    // Non-linear observer (see paper eqn 8):
    iq_t pm_flux_sqr_mf_2 = sq(config_.pm_flux_linkage * config_.freq);
    iq_t est_pm_flux_sqr_mf_2 = sq(eta_mf_[0]) + sq(eta_mf_[1]);
    iq_t eta_factor = 0.5_r * (config_.observer_gain / pm_flux_sqr_mf_2) * (pm_flux_sqr_mf_2 - est_pm_flux_sqr_mf_2);



    // alpha-beta vector operations
    for (size_t i = 0; i < 2; ++i) {
        // add observer action to flux estimate dynamics
        iq_t x_dot = eta_factor * eta_mf_[i];
        // convert action to discrete-time
        flux_state_mf_[i] += x_dot;
        // update new eta
        eta_mf_[i] = flux_state_mf_[i] - (config_.phase_inductance * config_.freq) * I_alpha_beta[i];
    }

    // Flux state estimation done, store V_alpha_beta for next timestep
    V_alpha_beta_last_[0] = Valpha;
    V_alpha_beta_last_[1] = Vbeta;

    // phase_ = atan2(eta_mf_[1], eta_mf_[0]);
    phase_ = atan2(-eta_mf_[0], eta_mf_[1]);
}

void Pll::update(const iq_t phase){
    // scexpr iq_t pll_kp = 0.17_r;
    // scexpr iq_t pll_ki = 0.0027_r;
    scexpr iq_t pll_kp = 0.17_r;
    scexpr iq_t pll_ki = 0.0027_r;

    const iq_t lap_pos = frac(phase * iq_t(1 / TAU) + 10); 
    iq_t delta_lap_pos = lap_pos - last_lap_pos;

    if(delta_lap_pos >= iq_t(0.5)){
        delta_lap_pos -= 1;
    }else if (delta_lap_pos <= -iq_t(0.5)){
        delta_lap_pos += 1;
    }

    accu_pos_ += delta_lap_pos;
    last_lap_pos = lap_pos;
    
    iq_t pos_err = accu_pos_ - pll_pos_;
    err_int_ += (pos_err);
    pll_pos_ += (err_int_ * pll_ki + pos_err * pll_kp);

}
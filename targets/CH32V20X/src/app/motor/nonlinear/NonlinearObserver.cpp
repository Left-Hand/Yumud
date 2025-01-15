#include "NonlinearObserver.hpp"

using namespace ymd::foc;

scexpr iq_t wrap_pm_pi(const iq_t x){
    return fposmodp(x, iq_t(TAU));
}
void NonlinearObserver::reset(){
    pll_pos_ = 0.0_r;
    V_alpha_beta_memory_[0] = 0.0_r;
    V_alpha_beta_memory_[1] = 0.0_r;
    flux_state_[0] = 0.0_r;
    flux_state_[1] = 0.0_r;
}

void NonlinearObserver::update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta){
    // Algorithm based on paper: Sensorless Control of Surface-Mount Permanent-Magnet Synchronous Motors Based on a Nonlinear Observer
    // http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
    // In particular, equation 8 (and by extension eqn 4 and 6).

    // The V_alpha_beta applied immedietly prior to the current measurement associated with this cycle
    // is the one computed two cycles ago. To get the correct measurement, it was stored twice:
    // once by final_v_alpha/final_v_beta in the current control reporting, and once by V_alpha_beta_memory.

    // PLL
    // TODO: the PLL part has some code duplication with the encoder PLL
    // Pll gains as a function of bandwidth
    iq_t pll_kp = 2 * config_.pll_bandwidth;
    // Critically damped
    iq_t pll_ki = 0.25_r * (pll_kp * pll_kp);

    // Check that we don't get problems with discrete time approximation
    if (!(config_.period * pll_kp < 1)) {
        HALT;
    }

    // Clarke transform
    iq_t I_alpha_beta[2] = {Ialpha, Ibeta};

    // alpha-beta vector operations
    iq_t eta[2];
    for (int i = 0; i <= 1; ++i) {
        // y is the total flux-driving voltage (see paper eqn 4)
        iq_t y = -config_.phase_resistance * I_alpha_beta[i] + V_alpha_beta_memory_[i];
        // flux dynamics (prediction)
        iq_t x_dot = y;
        // integrate prediction to current timestep
        flux_state_[i] += x_dot * config_.period;

        // eta is the estimated permanent magnet flux (see paper eqn 6)
        eta[i] = flux_state_[i] - config_.phase_inductance * I_alpha_beta[i];
    }

    // Non-linear observer (see paper eqn 8):
    iq_t pm_flux_sqr = config_.pm_flux_linkage * config_.pm_flux_linkage;
    iq_t est_pm_flux_sqr = eta[0] * eta[0] + eta[1] * eta[1];
    iq_t bandwidth_factor = 1.0_r / pm_flux_sqr;
    iq_t eta_factor = 0.5_r * (config_.observer_gain * bandwidth_factor) * (pm_flux_sqr - est_pm_flux_sqr);

    // alpha-beta vector operations
    for (int i = 0; i <= 1; ++i) {
        // add observer action to flux estimate dynamics
        iq_t x_dot = eta_factor * eta[i];
        // convert action to discrete-time
        flux_state_[i] += x_dot * config_.period;
        // update new eta
        eta[i] = flux_state_[i] - config_.phase_inductance * I_alpha_beta[i];
    }

    // Flux state estimation done, store V_alpha_beta for next timestep
    V_alpha_beta_memory_[0] = Valpha;
    V_alpha_beta_memory_[1] = Vbeta;

    iq_t phase_vel = phase_vel_;

    // predict PLL phase with velocity
    pll_pos_ = wrap_pm_pi(pll_pos_ + config_.period * phase_vel);
    // update PLL phase with observer permanent magnet phase
    iq_t phase = atan2(eta[1], eta[0]);
    iq_t delta_phase = wrap_pm_pi(phase - pll_pos_);
    pll_pos_ = wrap_pm_pi(pll_pos_ + config_.period * pll_kp * delta_phase);
    // update PLL velocity
    phase_vel += config_.period * pll_ki * delta_phase;

    // set outputs
    phase_ = phase;
    phase_vel_ = phase_vel;

}
#pragma once

#include "core/math/real.hpp"

// http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
// https://www.bilibili.com/video/BV1hmtQzJEBf

namespace ymd::dsp::motor_ctl{


class NonlinearFluxObserver final{
public:
    struct Config{
        q16 phase_inductance;
        q16 phase_resistance;
        q16 observer_gain; // [rad/s]
        q16 pm_flux_linkage; // [V / (rad/s)]
        size_t freq;
    };

public:
    constexpr explicit NonlinearFluxObserver(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        phase_resistance_ = cfg.phase_resistance;
        pm_flux_sqr_mf_2_ = square(cfg.pm_flux_linkage * cfg.freq);
        temp1_ = (cfg.observer_gain / pm_flux_sqr_mf_2_);
        phase_inductance_mul_config_freq_ = (cfg.phase_inductance * cfg.freq);
    }

    constexpr void reset(){
        V_alpha_beta_last_[0] = 0;
        V_alpha_beta_last_[1] = 0;
        flux_state_mf_[0] = 0;
        flux_state_mf_[1] = 0;
    }


    constexpr void update(auto alpha_beta_volt, auto alpha_beta_curr){
        // Algorithm based on paper: Sensorless Control of Surface-Mount Permanent-Magnet Synchronous Motors Based on a Nonlinear Observer
        // http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
        // In particular, equation 8 (and by extension eqn 4 and 6).

        // The V_alpha_beta applied immedietly prior to the current measurement associated with this cycle
        // is the one computed two cycles ago. To get the correct measurement, it was stored twice:
        // once by final_v_alpha/final_v_beta in the current control reporting, and once by V_alpha_beta_memory.
        const auto [Valpha, Vbeta] = alpha_beta_volt;
        const auto [Ialpha, Ibeta] = alpha_beta_curr;
        const q16 I_alpha_beta[2] = {Ialpha, Ibeta};
        // alpha-beta vector operations
        q16 eta_mf[2];

        #pragma GCC unroll 2
        for (size_t i = 0; i < 2; ++i) {
            // flux dynamics (prediction)
            q16 x_dot = -phase_resistance_ * I_alpha_beta[i] + V_alpha_beta_last_[i];
            // integrate prediction to current timestep
            flux_state_mf_[i] += x_dot;

            // eta is the estimated permanent magnet flux (see paper eqn 6)
            eta_mf[i] = flux_state_mf_[i] - phase_inductance_mul_config_freq_ * I_alpha_beta[i];
        }

        // Non-linear observer (see paper eqn 8):

        q16 est_pm_flux_sqr_mf_2 = square(eta_mf[0]) + square(eta_mf[1]);
        q16 eta_factor = temp1_ * (pm_flux_sqr_mf_2_ - est_pm_flux_sqr_mf_2) >> 1;



        // alpha-beta vector operations
        #pragma GCC unroll 2
        for (size_t i = 0; i < 2; ++i) {
            // add observer action to flux estimate dynamics
            q16 x_dot = eta_factor * eta_mf[i];
            // convert action to discrete-time
            flux_state_mf_[i] += x_dot;
            // update new eta
            eta_mf[i] = flux_state_mf_[i] - phase_inductance_mul_config_freq_ * I_alpha_beta[i];
        }

        // Flux state estimation done, store V_alpha_beta for next timestep
        V_alpha_beta_last_[0] = Valpha;
        V_alpha_beta_last_[1] = Vbeta;

        // phase_ = atan2(eta_mf[1], eta_mf[0]);
        turns_ = atan2pu(eta_mf[1], eta_mf[0]);
    }

    constexpr Angle<q16> angle() const {
        return Angle<q16>::from_turns(frac(q16(turns_)));
    }
private:
    // Config config_;
    q16 phase_resistance_;
    q16 temp1_;
    q16 pm_flux_sqr_mf_2_;
    q16 phase_inductance_mul_config_freq_;
    q16 flux_state_mf_[2] = {0, 0};        // [Vs * Fs]
    q16 V_alpha_beta_last_[2] = {0, 0}; // [V]
    q16 turns_ = 0;                   // [rad]
};


}
#pragma once

#include "core/math/real.hpp"

// http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
// https://www.bilibili.com/video/BV1hmtQzJEBf

namespace ymd::dsp::motor_ctl{


class NonlinearFluxObserver final{
public:
    struct Config{
        uint32_t fs;
        q20 phase_inductance;
        q20 phase_resistance;
        q20 observer_gain; // [rad/s]
        q20 pm_flux_linkage; // [V / (rad/s)]

    };

public:
    constexpr explicit NonlinearFluxObserver(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        phase_resistance_ = cfg.phase_resistance;
        pm_flux_sqr_mf_2_ = square(cfg.pm_flux_linkage * cfg.fs);
        temp1_ = (cfg.observer_gain / pm_flux_sqr_mf_2_);
        phase_inductance_mul_config_freq_ = (cfg.phase_inductance * cfg.fs);
    }

    constexpr void reset(){
        V_alphabeta_last_[0] = 0;
        V_alphabeta_last_[1] = 0;
        flux_state_mf_[0] = 0;
        flux_state_mf_[1] = 0;
    }


    constexpr void update(auto alphabeta_volt, auto alphabeta_curr){
        // Algorithm based on paper: Sensorless Control of Surface-Mount Permanent-Magnet Synchronous Motors Based on a Nonlinear Observer
        // http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
        // In particular, equation 8 (and by extension eqn 4 and 6).

        // The V_alphabeta applied immedietly prior to the current measurement associated with this cycle
        // is the one computed two cycles ago. To get the correct measurement, it was stored twice:
        // once by final_v_alpha/final_v_beta in the current control reporting, and once by V_alphabeta_memory.
        const auto [Valpha, Vbeta] = alphabeta_volt;
        const auto [Ialpha, Ibeta] = alphabeta_curr;
        const q20 I_alphabeta[2] = {Ialpha, Ibeta};
        // alpha-beta vector operations
        q20 eta_mf[2];

        #pragma GCC unroll 2
        for (size_t i = 0; i < 2; ++i) {
            // flux dynamics (prediction)
            q20 x_dot = -phase_resistance_ * I_alphabeta[i] + V_alphabeta_last_[i];
            // integrate prediction to current timestep
            flux_state_mf_[i] += x_dot;

            // eta is the estimated permanent magnet flux (see paper eqn 6)
            eta_mf[i] = flux_state_mf_[i] - phase_inductance_mul_config_freq_ * I_alphabeta[i];
        }

        // Non-linear observer (see paper eqn 8):

        q20 est_pm_flux_sqr_mf_2 = square(eta_mf[0]) + square(eta_mf[1]);
        q20 eta_factor = temp1_ * (pm_flux_sqr_mf_2_ - est_pm_flux_sqr_mf_2) >> 1;



        // alpha-beta vector operations
        #pragma GCC unroll 2
        for (size_t i = 0; i < 2; ++i) {
            // add observer action to flux estimate dynamics
            q20 x_dot = eta_factor * eta_mf[i];
            // convert action to discrete-time
            flux_state_mf_[i] += x_dot;
            // update new eta
            eta_mf[i] = flux_state_mf_[i] - phase_inductance_mul_config_freq_ * I_alphabeta[i];
        }

        // Flux state estimation done, store V_alphabeta for next timestep
        V_alphabeta_last_[0] = Valpha;
        V_alphabeta_last_[1] = Vbeta;

        // phase_ = atan2(eta_mf[1], eta_mf[0]);
        turns_ = atan2pu(eta_mf[1], eta_mf[0]);
    }

    constexpr Angle<iq16> angle() const {
        return Angle<iq16>::from_turns(frac(q16(turns_)));
    }
// private:
public:
    // Config config_;
    q20 phase_resistance_;
    q20 temp1_;
    q20 pm_flux_sqr_mf_2_;
    q20 phase_inductance_mul_config_freq_;
    q20 flux_state_mf_[2] = {0, 0};        // [Vs * Fs]
    q20 V_alphabeta_last_[2] = {0, 0}; // [V]
    q20 turns_ = 0;                   // [rad]
};


}
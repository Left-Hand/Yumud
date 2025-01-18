#pragma once

#include "sys/math/real.hpp"

namespace ymd::foc{
class NonlinearObserver {
public:
    struct Config{
        iq_t phase_inductance;
        iq_t phase_resistance;
        iq_t observer_gain = 1000.0_r; // [rad/s]
        iq_t pll_bandwidth = 1000.0_r;  // [rad/s]
        iq_t pm_flux_linkage = 1.58e-3_r; // [V / (rad/s)]  { 5.51328895422 / (<pole pairs> * <rpm/v>) }
        iq_t period = 0.0001_r;
    };
protected:
    // TODO: expose on protocol
    iq_t pll_pos_ = 0.0_r;                      // [rad]
    iq_t flux_state_[2] = {0.0_r, 0.0_r};        // [Vs]
    iq_t V_alpha_beta_last_[2] = {0.0_r, 0.0_r}; // [V]

    iq_t phase_ = 0.0_r;                   // [rad]
    iq_t phase_vel_ = 0.0_r;               // [rad/s]
    const Config & config_;
public:
    NonlinearObserver(const Config & config):
        config_(config){;}

    void reset();
    void init();
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);
};

class NonlinearObserver2{
public:
    struct Config{
        iq_t phase_inductance;
        iq_t phase_resistance;
        iq_t observer_gain; // [rad/s]
        iq_t pm_flux_linkage; // [V / (rad/s)]
        size_t freq;
    };
protected:
    const Config & config_;
    iq_t flux_state_mul_freq[2];        // [Vs * Fs]
    iq_t V_alpha_beta_last_[2]; // [V]
    iq_t phase_;                   // [rad]
public:
    NonlinearObserver2(const Config & config):
        config_(config)
    {
        reset();
    }

    void reset();
    void init();
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);
};
}
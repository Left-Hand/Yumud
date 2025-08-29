#pragma once

#include "core/math/real.hpp"
#include "Pll.hpp"


namespace ymd::foc{
class NonlinearObserver final{
public:
    struct Config{
        q16 phase_inductance;
        q16 phase_resistance;
        q16 observer_gain; // [rad/s]
        q16 pm_flux_linkage; // [V / (rad/s)]
        size_t freq;
    };

public:
    NonlinearObserver(const Config & config):
        config_(config)
    {
        reset();
    }

    void reset();
    void reconf(const Config & config){config_ = config;}
    void update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta);

    q16 theta() const {return phase_;}
private:
    Config config_;
    q16 flux_state_mf_[2] = {0, 0};        // [Vs * Fs]
    q16 V_alpha_beta_last_[2] = {0, 0}; // [V]
    q16 phase_ = 0;                   // [rad]
};


}
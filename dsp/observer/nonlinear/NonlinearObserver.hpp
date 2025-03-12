#pragma once

#include "sys/math/real.hpp"
#include "Pll.hpp"


namespace ymd::foc{
class NonlinearObserver{
public:
    struct Config{
        iq_t<16> phase_inductance;
        iq_t<16> phase_resistance;
        iq_t<16> observer_gain; // [rad/s]
        iq_t<16> pm_flux_linkage; // [V / (rad/s)]
        size_t freq;
    };
protected:
    Config config_;
    iq_t<16> flux_state_mf_[2];        // [Vs * Fs]
    iq_t<16> V_alpha_beta_last_[2]; // [V]
    iq_t<16> phase_;                   // [rad]
public:
    NonlinearObserver(const Config & config):
        config_(config)
    {
        reset();
    }

    void reset();
    void reconfig(const Config & config){config_ = config;}
    void init();
    void update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta);

    iq_t<16> theta() const {return phase_;}
};


}
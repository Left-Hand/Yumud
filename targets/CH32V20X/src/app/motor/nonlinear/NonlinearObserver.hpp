#pragma once

#include "sys/math/real.hpp"
#include "../SensorlessObserverIntf.hpp"
#include "Pll.hpp"


namespace ymd::foc{
class NonlinearObserver:public SensorlessObserverIntf{
public:
    struct Config{
        iq_t phase_inductance;
        iq_t phase_resistance;
        iq_t observer_gain; // [rad/s]
        iq_t pm_flux_linkage; // [V / (rad/s)]
        size_t freq;
    };
protected:
    Config config_;
    iq_t flux_state_mf_[2];        // [Vs * Fs]
    iq_t V_alpha_beta_last_[2]; // [V]
    iq_t phase_;                   // [rad]
public:
    NonlinearObserver(const Config & config):
        config_(config)
    {
        reset();
    }

    void reset();
    void reconfig(const Config & config){config_ = config;}
    void init();
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);

    iq_t theta() const {return phase_;}
};


}
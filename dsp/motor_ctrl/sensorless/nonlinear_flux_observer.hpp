#pragma once

#include "core/math/real.hpp"
#include "core/string/view/string_view.hpp"

// http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
// https://www.bilibili.com/video/BV1hmtQzJEBf

namespace ymd::dsp::motor_ctl{


class NonlinearFluxObserver final{
public:
    struct [[nodiscard]] Coeffs final{
        iq16 phase_resistance;
        iq20 temp1;
        iq12 pm_flux_sqr_mf_2;
        iq16 phase_inductance_mf;
    };

    struct [[nodiscard]] Config final{
        uint32_t fs;
        iq20 phase_inductance;
        iq16 phase_resistance;
        iq20 observer_gain; // [rad/s]
        iq20 pm_flux_linkage; // [V / (rad/s)]

        constexpr Result<Coeffs, StringView> try_into_coeffs() const {
            auto & cfg = *this;
            const iq12 pm_flux_sqr_mf_2 = math::square(iq12(cfg.pm_flux_linkage * cfg.fs));
            const iq20 temp1 = (cfg.observer_gain / pm_flux_sqr_mf_2);
            const iq16 phase_inductance_mf = (cfg.phase_inductance * cfg.fs);
            
            
            Coeffs coeffs{
                .phase_resistance = phase_resistance,
                .temp1 = temp1,
                .pm_flux_sqr_mf_2 = pm_flux_sqr_mf_2,
                .phase_inductance_mf = phase_inductance_mf
            };
            return Ok(coeffs);
        }
    };

    struct [[nodiscard]] ConfigF32 final{
        uint32_t fs;
        float phase_inductance;
        float phase_resistance;
        float observer_gain; // [rad/s]
        float pm_flux_linkage; // [V / (rad/s)]

        consteval Result<Coeffs, StringView> try_into_coeffs() const {
            auto & cfg = *this;
            const float pm_flux_sqr_mf_2 = math::square((cfg.pm_flux_linkage * cfg.fs));
            const float temp1 = (cfg.observer_gain / pm_flux_sqr_mf_2);
            const float phase_inductance_mf = (cfg.phase_inductance * cfg.fs);
            
            
            Coeffs coeffs;
            coeffs.phase_resistance = coeffs.phase_resistance.from(phase_resistance);
            coeffs.temp1 = coeffs.temp1.from(temp1);
            coeffs.pm_flux_sqr_mf_2 = coeffs.pm_flux_sqr_mf_2.from(pm_flux_sqr_mf_2);
            coeffs.phase_inductance_mf = coeffs.phase_inductance_mf.from(phase_inductance_mf);

            return Ok(coeffs);
        }
    };

    struct [[nodiscard]] State final{
        std::array<iq16, 2> flux_state_mf;        // [Vs * Fs]
        std::array<iq16, 2> v_alphabeta_last; // [V]
        uq32 turns;                   // [rad]

        static constexpr State zero() {
            return State{
                .flux_state_mf = {0, 0},
                .v_alphabeta_last = {0, 0},
                .turns = 0
            };
        }

        static constexpr State from_default() {
            return zero();
        }
    };

public:
    constexpr explicit NonlinearFluxObserver(const Coeffs & coeffs){
        this->coeffs_ = coeffs;
        reset();
    }


    constexpr void reset(){
        state_ = State::zero();
    }

    template<size_t Q0, size_t Q1, size_t Q2>
    static constexpr math::fixed<Q0, int32_t> fixed_mul(math::fixed<Q1, int32_t> a, math::fixed<Q2, int32_t> b){ 
        const int32_t bits = static_cast<int32_t>((static_cast<int64_t>(a.to_bits()) * static_cast<int64_t>(b.to_bits())) >> (Q1 + Q2 - Q0));
        return math::fixed<Q0, int32_t>::from_bits(bits);
    }

    // template<size_t Q0, size_t Q1, size_t Q2>
    // static constexpr math::fixed<Q0, int32_t> fixed_mul(math::fixed<Q1, uint32_t> a, math::fixed<Q2, int32_t> b){ 
    //     const int32_t bits = static_cast<int32_t>((static_cast<int64_t>(a.to_bits()) * static_cast<int64_t>(b.to_bits())) >> (Q1 + Q2 - Q0));
    //     return math::fixed<Q0, int32_t>::from_bits(bits);
    // }

    constexpr void update(auto && alphabeta_volt, auto && alphabeta_curr){
        // Algorithm based on paper: Sensorless Control of Surface-Mount Permanent-Magnet Synchronous Motors Based on a Nonlinear Observer
        // http://cas.ensmp.fr/~praly/Telechargement/Journaux/2010-IEEE_TPEL-Lee-Hong-Nam-Ortega-Praly-Astolfi.pdf
        // In particular, equation 8 (and by extension eqn 4 and 6).

        // The V_alphabeta applied immedietly prior to the current measurement associated with this cycle
        // is the one computed two cycles ago. To get the correct measurement, it was stored twice:
        // once by final_v_alpha/final_v_beta in the current control reporting, and once by V_alphabeta_memory.

        const iq16 I_alphabeta[2] = {
            static_cast<iq16>((alphabeta_curr)[0]), 
            static_cast<iq16>((alphabeta_curr)[1]), 
        };
        // alpha-beta vector operations
        iq16 eta_mf[2];

        #pragma GCC unroll 2
        for (size_t i = 0; i < 2; ++i) {
            // flux dynamics (prediction)
            iq16 x_dot = -coeffs_.phase_resistance * I_alphabeta[i] + state_.v_alphabeta_last[i];
            // integrate prediction to current timestep
            state_.flux_state_mf[i] += x_dot;

            // eta is the estimated permanent magnet flux (see paper eqn 6)
            eta_mf[i] = state_.flux_state_mf[i] - coeffs_.phase_inductance_mf * I_alphabeta[i];
        }

        // Non-linear observer (see paper eqn 8):

        iq12 est_pm_flux_sqr_mf_2 = math::square(static_cast<iq12>(eta_mf[0])) + math::square(static_cast<iq12>(eta_mf[1]));
        const auto eta_factor = fixed_mul<16>(coeffs_.temp1, ((coeffs_.pm_flux_sqr_mf_2 - est_pm_flux_sqr_mf_2) >> 1));



        // alpha-beta vector operations
        #pragma GCC unroll 2
        for (size_t i = 0; i < 2; ++i) {
            // add observer action to flux estimate dynamics
            iq16 x_dot = eta_factor * eta_mf[i];
            // convert action to discrete-time
            state_.flux_state_mf[i] += x_dot;
            // update new eta
            eta_mf[i] = state_.flux_state_mf[i] - coeffs_.phase_inductance_mf * I_alphabeta[i];
        }

        // Flux state estimation done, store V_alphabeta for next timestep
        state_.v_alphabeta_last[0] = static_cast<iq16>((alphabeta_volt)[0]);
        state_.v_alphabeta_last[1] = static_cast<iq16>((alphabeta_volt)[1]);

        // phase_ = atan2(eta_mf[1], eta_mf[0]);
        state_.turns = math::atan2pu(eta_mf[1], eta_mf[0]);
    }

    constexpr Angular<uq32> angle() const {
        return Angular<uq32>::from_turns(state_.turns);
    }

    constexpr const State & state() const {
        return state_;
    }

    constexpr const Coeffs & coeffs() const {
        return coeffs_;
    }
// private:
public:
    Coeffs coeffs_;
    State state_;
};


}
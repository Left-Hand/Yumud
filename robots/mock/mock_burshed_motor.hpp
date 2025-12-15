#pragma once

#include "core/math/realmath.hpp"
#include "dsp/state_vector.hpp"

namespace ymd::robots::mock{
struct MockBrushedMotor final{
public:
    using State = dsp::StateVector<iq16, 2>;
    struct Config{
        uint32_t fs;
    };

    MockBrushedMotor(const Config & cfg){reconf(cfg);}

    void reconf(const Config & cfg){
        dt_ = 1_r / cfg.fs;
    }

    constexpr State update(const State & state, const iq16 u_in) const {
        const auto & self = *this;
        // const auto distrb_of_partial = 0;
        // constexpr auto DAMPING = 0;
        // const auto distrb_of_damp = DAMPING * state[1];
        // const auto distrb_of_partial = math::sinpu(state[0]);
        // const auto u = u_in + distrb_of_partial - distrb_of_damp;
        // const auto u = STEP_TO(u_in + distrb_of_partial - distrb_of_damp, q16(0), 100_iq16);
        // const auto u = STEP_TO(u_in + distrb_of_partial - distrb_of_damp, q16(0;
        const auto u = u_in;
        return State{
            state[0] + state[1] * self.dt_,
            state[1] + u * self.dt_
        };
    }
private:
    using Self = MockBrushedMotor;


    State state_;
    iq16 dt_;

};

}
#pragma once

#include "core/math/realmath.hpp"
#include "dsp/state_vector.hpp"

namespace ymd::robots::mock{
struct MockBrushedMotor final{
public:
    struct Config{
        uint32_t fs;
    };

    MockBrushedMotor(const Config & cfg){reconf(cfg);}

    void reconf(const Config & cfg){
        dt_ = 1_r / cfg.fs;
    }

    void update(const real_t u){
        auto & self = *this;
        self.state_ = forward(self, state_, u);
    }

    const auto & state() const {return state_;}
private:
    using Self = MockBrushedMotor;
    using State = dsp::StateVector<real_t, 2>;

    State state_;
    real_t dt_;
    static constexpr State forward(const Self & self, const State & x, const real_t u_in){
        // const auto distrb_of_partial = 0;
        // constexpr auto DAMPING = 0;
        // const auto distrb_of_damp = DAMPING * x[1];
        // const auto distrb_of_partial = sinpu(x[0]);
        // const auto u = u_in + distrb_of_partial - distrb_of_damp;
        // const auto u = STEP_TO(u_in + distrb_of_partial - distrb_of_damp, q16(0), 100_iq16);
        // const auto u = STEP_TO(u_in + distrb_of_partial - distrb_of_damp, q16(0;
        const auto u = u_in;
        return {
            x[0] + x[1] * self.dt_,
            x[1] + u * self.dt_
        };
    }
};

}
#pragma once

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

#include "digipw/prelude/abdq.hpp"

// 全面推导龙伯格观测器相关公式
// https://zhuanlan.zhihu.com/p/960435762

namespace ymd::dsp::motor_ctl {

class LuenbergerObserver{
public:

    struct Config{
        uint32_t fs;
        // iq20 R = 10.0;
        // iq20 L = 2.64E-3;
        // int32_t l1 = -22000;
        // int32_t l2 = 64000;
        iq20 phase_inductance;
        iq20 phase_resistance;
    };

    struct State{
        digipw::AlphaBetaCoord<iq20> i;
        digipw::AlphaBetaCoord<iq20> e;

        constexpr void reset(){
            i = digipw::AlphaBetaCoord<iq20>(0, 0);
            e = digipw::AlphaBetaCoord<iq20>(0, 0);
        }

        constexpr Angular<uq32> angle() const noexcept {
            return Angular<uq32>::from_turns(math::atan2pu(e.beta, e.alpha));
        }
    };

    struct Coeffs{
        iq20 Tr_L;
        iq20 T_L ;
        iq20 l1T ;
        iq20 l2T ;

        static constexpr Coeffs from(const Config & cfg){
            //exp 43
            Coeffs self;
            const auto R = cfg.phase_resistance;
            const auto L = cfg.phase_inductance;
            const auto dt = uq32::from_rcp(cfg.fs);

            self.Tr_L = iq20(- R / L) * dt;
            self.T_L  = iq20(L) * dt;
            self.l1T  = iq20(R / L) * dt - 2;
            self.l2T  = iq20(L * cfg.fs);

            return self;
        }

        constexpr void iterate(State & state, const State & meas) const noexcept {
            //exp 43
    
            const auto err_i = state.i - meas.i;
            state.i += Tr_L * state.i + T_L * (meas.e - state.e) + l1T * err_i; 
            state.e += l2T * err_i;
        }
    };



};


}
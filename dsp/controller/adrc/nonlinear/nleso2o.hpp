#pragma once
#include "nleso_prelude.hpp"
namespace ymd::dsp::adrc{

//非线性拓展状态观测器
template<>
struct NonlinearExtendedStateObserver<iq16, 2>{
    using State = StateVector<iq20, 3>;
    constexpr State iterate(State state, const iq16 u, const iq16 y) const {
        const iq16 e = z1 - y;


        const iq16 z1  = state[0];
        const iq16 z2  = state[1];
        const iq16 z3  = state[2];

        state[0] = z1 + h *(z2-belta01*e);
        state[1] = z2 + h *(z3-belta02*fal(e,0.5,delta)+b*u);
        state[2] = z3 + h *(-belta03*fal(e,0.25,delta));
        return state;
    }
private:

}
}
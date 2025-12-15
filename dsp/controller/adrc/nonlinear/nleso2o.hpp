#pragma once
#include "nleso_prelude.hpp"
namespace ymd::dsp::adrc{

//非线性拓展状态观测器
template<>
struct NonlinearExtendedStateObserver<iq16, 2>{

    void update(const iq16 u, const iq16 y){
        const iq16 e = z1 - y;


        const iq16 z1  = state_[0];
        const iq16 z2  = state_[1];
        const iq16 z3  = state_[2];

        state_[0] = z1 + h *(z2-belta01*e);
        state_[1] = z2 + h *(z3-belta02*fal(e,0.5,delta)+b*u);
        state_[2] = z3 + h *(-belta03*fal(e,0.25,delta));
    }
private:
    using State = StateVector<iq20, 3>;
    State state_;
}
}
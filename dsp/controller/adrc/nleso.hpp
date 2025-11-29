#pragma once

namespace ymd::dsp::adrc{


// class ExtendedStateObserver{

//     void update(const real_t u, const real_t y){
//         const real_t e = z1 - y;


//         const real_t z1  = state_[0];
//         const real_t z2  = state_[1];
//         const real_t z3  = state_[2];

//         state_[0] = z1 + h *(z2-belta01*e);
        // state_[1] = z2 + h *(z3-belta02*fal(e,0.5,delta)+b*u);
//         state_[2] = z3 + h *(-belta03*fal(e,0.25,delta));
//     }
// private:
//     using State = StateVector<iq20, 3>;
//     State state_;
// }
}
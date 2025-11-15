#pragma once

#include "core/math/iq/iqmath.hpp"
#include "core/math/real.hpp"

#include "dsp/state_vector.hpp"

namespace ymd::dsp::adrc{

static constexpr 
iq16 fal(const iq16 e, const iq16 alpha, const iq16 delta){

    const auto fabsf_e = std::abs(e);
    
    if(delta>=fabsf_e)
        return e/pow(delta,1-alpha);
    else
        return pow(fabsf_e,alpha)*sign(e);
}

static constexpr 
iq16 ssqrt(iq16 x){
    return sign(x) * sqrt(std::abs(x));
}




}
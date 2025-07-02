#pragma once

#include "core/math/iq/iqmath.hpp"
#include "core/math/real.hpp"

#include "dsp/state_vector.hpp"

namespace ymd::dsp::adrc{

static constexpr 
real_t fal(const real_t e, const real_t alpha, const real_t delta){

    const auto fabsf_e = abs(e);
    
    if(delta>=fabsf_e)
        return e/pow(delta,1-alpha);
    else
        return pow(fabsf_e,alpha)*sign(e);
}

static constexpr 
real_t ssqrt(real_t x){
    return sign(x) * sqrt(ABS(x));
}




}
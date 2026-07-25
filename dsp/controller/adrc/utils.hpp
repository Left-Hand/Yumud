#pragma once

#include "core/math/fixed/fxmath.hpp"
#include "core/math/real.hpp"

#include "dsp/state_vector.hpp"

namespace ymd::dsp::adrc{

static constexpr 
iq16 fal(const iq16 e, const iq16 alpha, const uq16 delta){

    const uq16 fabsf_e = std::abs(e);
    
    if(delta>=fabsf_e)
        return e/math::pow(delta,1-alpha);
    else
        return math::pow(fabsf_e,alpha)*math::sign(e);
}

static constexpr 
iq16 sat_sqrt(const iq16 d, const iq16 _8r, const iq16 abs_y){
    uint64_t uuq32_sum_bits = 
        uint64_t((static_cast<int64_t>(d.to_bits())) * (static_cast<int64_t>(d.to_bits()))) + 
        uint64_t(static_cast<int64_t>(_8r.to_bits()) * static_cast<int64_t>(abs_y.to_bits()));
    // if(uuq32_sum_bits >= std::numeric_limits<uint32_t>::max()){
    //     return 1 << ((32 - 16) / 2);
    // }
    return math::sqrt(uuq16::from_bits(uuq32_sum_bits >> 16));
}

// fhan函子
template<typename T>
struct FhanPrecomputed;

template<>
struct [[nodiscard]] FhanPrecomputed<iq16>{
    struct [[nodiscard]] Config{
        uq16 r;
        uq16 h;
    };

    constexpr explicit FhanPrecomputed(const Config & cfg):
        r(cfg.r),
        h(cfg.h),
        d(cfg.r * cfg.h),
        d0(iq16(cfg.r * cfg.h) * cfg.h),
        inv_h(1 / cfg.h),
        inv_d(1 / iq16(cfg.r * cfg.h)){;}

    [[nodiscard]] constexpr iq16 operator()(
        const std::array<iq16, 2> e
    ) const noexcept {
        const auto [e1, e2] = e;
        const iq16 y = -e1 - e2 * h;
        const iq16 abs_y = math::abs(y);
        const iq16 a0 = sat_sqrt(d, 8 * r, abs_y);
        
        const iq16 a = [&]{
            if(abs_y > d0){
                if(y > 0)
                    return -e2 + ((a0 - d) >> 1);
                else 
                    return -e2 - ((a0 - d) >> 1);
            }else{
                return -e2 + y * inv_h;
            }
        }();


        if(math::abs(a) > d){
            if(a > 0) 
                return  -r;
            else 
                return r;
        }else{
            return -r * (a * inv_d);
        }

    }

    iq16 r;
    iq16 h;
    iq16 d;
    iq16 d0;
    uq16 inv_h;
    uq16 inv_d;
};



}
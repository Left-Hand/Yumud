#pragma once

#include "core/math/iq/iqmath.hpp"
#include "core/math/real.hpp"

#include "dsp/state_vector.hpp"

namespace ymd::dsp::adrc{

static constexpr 
iq16 fal(const iq16 e, const iq16 alpha, const iq16 delta){

    const auto fabsf_e = std::abs(e);
    
    if(delta>=fabsf_e)
        return e/math::pow(delta,1-alpha);
    else
        return math::pow(fabsf_e,alpha)*math::sign(e);
}

static constexpr 
iq16 sat_sqrt(const iq16 d, const iq16 _8r, const iq16 abs_y){
    uint64_t uuq32_sum_bits = 
        ((static_cast<int64_t>(d.to_bits())) * (static_cast<int64_t>(d.to_bits()))) + 
        (static_cast<int64_t>(_8r.to_bits()) * static_cast<int64_t>(abs_y.to_bits()));
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
        r_(cfg.r),
        h_(cfg.h),
        d_(cfg.r * cfg.h),
        d0_(iq16(cfg.r * cfg.h) * cfg.h),
        inv_h_(1 / cfg.h),
        inv_d_(1 / iq16(cfg.r * cfg.h)){;}

    [[nodiscard]] constexpr iq16 operator()(
        const std::array<iq16, 2> e
    ) const{
        const auto [e1, e2] = e;
        const iq16 y = -e1 - e2 * h_;
        const iq16 abs_y = math::abs(y);
        const iq16 a0 = sat_sqrt(d_, 8 * r_, abs_y);
        
        const iq16 a = [&]{
            if(abs_y > d0_){
                if(y > 0)
                    return -e2 + ((a0 - d_) >> 1);
                else 
                    return -e2 - ((a0 - d_) >> 1);
            }else{
                return -e2 + y * inv_h_;
            }
        }();


        if(math::abs(a) > d_){
            if(a > 0) 
                return  -r_;
            else 
                return r_;
        }else{
            return -r_ * (a * inv_d_);
        }

    }
private:
    iq16 r_;
    iq16 h_;
    iq16 d_;
    iq16 d0_;
    uq16 inv_h_;
    uq16 inv_d_;


};



}
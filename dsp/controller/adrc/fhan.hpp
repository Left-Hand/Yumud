#pragma once

#include "core/math/realmath.hpp"

namespace ymd::dsp{
template<typename T>
struct FhanPrecomputed{
    struct Config{
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
        const iq16 e1, 
        const iq16 e2
    ) const{
        const iq16 y = -e1 - e2 * h_;//var
        const iq16 abs_y = ABS(y);
        const iq16 a0 = sat_sqrt(iq8(d_), iq8(8 * r_), abs_y);//var
        
        const iq16 a = [&]{
            if(abs_y > d0_){
                if(y > 0)
                    return -e2 + ((a0 - d_) >> 1);//var
                else 
                    return -e2 - ((a0 - d_) >> 1);//var
            }else{
                return -e2 + y * inv_h_;//var
            }
        }();


        if(ABS(a) > d_){
            if(a > 0) 
                return  -r_;//var
            else 
                return r_;//var
        }else{
            return -r_ * (a * inv_d_);//var
        }

    }
private:
    iq16 r_;
    iq16 h_;
    iq16 d_;
    iq16 d0_;
    uq16 inv_h_;
    uq16 inv_d_;

    __fast_inline static constexpr iq16
    sat_sqrt(const iq8 d, const iq8 _8r, const iq8 abs_y){
        uint64_t sum_bits_q16 = 
            (square(static_cast<int64_t>(d.to_bits()))) + 
            (static_cast<int64_t>(_8r.to_bits()) * static_cast<int64_t>(abs_y.to_bits()));
        // sum_bits_q16>>=8;
        if(sum_bits_q16 >= std::numeric_limits<uint32_t>::max()){
            return 1 << ((32 - 16) / 2);
        }
        return sqrt(iq16::from_bits(static_cast<int32_t>(sum_bits_q16)));
    }
};


}
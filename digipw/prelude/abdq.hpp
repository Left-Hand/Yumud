#pragma once

#include "uvw.hpp"


namespace ymd::digipw{
struct AlphaBetaDuty{
    q16 alpha;
    q16 beta;

    q16 & operator [](size_t idx){
        switch(idx){
            case 0: return alpha;
            case 1: return beta;
            default: __builtin_unreachable();
        }
    }


    const q16 & operator [](size_t idx) const{
        switch(idx){
            case 0: return alpha;
            case 1: return beta;
            default: __builtin_unreachable();
        }
    }
};


struct DqValue{
    q20 d, q;

    constexpr q20 operator [](const size_t idx) const {
        return *(&d + idx);
    }

    constexpr q20 & operator [](const size_t idx){
        return *(&d + idx);
    }

    constexpr q20 length() const {
        return mag(d,q);
    }

    friend OutputStream & operator << (OutputStream & os, const DqValue & self){
        return os << os.brackets<'('>() << 
            self.d << os.splitter() << 
            self.q << os.brackets<')'>();
    }
};



struct AbValue{
    q20 a = {};
    q20 b = {};

    constexpr q20 operator [](const size_t idx) const {
        return *(&a + idx);
    }

    constexpr q20 & operator [](const size_t idx){
        return *(&a + idx);
    }

    constexpr q20 length() const {
        return sqrt(a*a + b*b);
    }

    friend OutputStream & operator << (OutputStream & os, const AbValue & self){
        return os << os.brackets<'('>() << 
            self.a << os.splitter() << 
            self.b << os.brackets<')'>();
    }
};



namespace details{
static constexpr __fast_inline void __ab_to_dq(DqValue & dq, const AbValue & ab, const q16 rad){
    auto [s,c] = sincos(rad);
    dq = {s * ab[1] + c * ab[0], c * ab[1] - s * ab[0]};
};

static constexpr __fast_inline void __dq_to_ab(AbValue & ab, const DqValue & dq, const q16 rad){
    auto [s,c] = sincos(rad);
    ab = {c * dq[0] - s * dq[1], c * dq[1] + s * dq[0]};
};
}


struct AbCurrent: public AbValue{
    static constexpr AbCurrent from_uvw(const UvwCurrent & uvw){
        constexpr auto _2_by_3 = q20(2.0/3);
        constexpr auto _sqrt3_by_3 = q20(sqrt(q20(3)) / 3);
        return {(uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, (uvw.v - uvw.w) * _sqrt3_by_3};
    };
    // constexpr AbCurrent to_alpha_beta() const {
    //     AbCurrent ab;
    //     return details::__dq_to_ab(ab, *this)
    // }
};
struct AbVoltage: public AbValue{
    // constexpr AbCurrent to_alpha_beta() const {
    //     AbCurrent ab;
    //     return details::__dq_to_ab(ab, *this)
    // }
};




struct DqCurrent: public DqValue{
    static constexpr DqCurrent from_alpha_beta(const AbCurrent & ab, const q16 rad){
        DqCurrent self;
        details::__ab_to_dq(self, ab, rad);
        return self;
    }
    constexpr AbCurrent to_alpha_beta(const q16 rad) const {
        AbCurrent ab;
        details::__dq_to_ab(ab, *this, rad);
        return ab;
    }
};
struct DqVoltage: public DqValue{
    static constexpr DqVoltage from_alpha_beta(const AbVoltage & ab, const q16 rad){
        DqVoltage self;
        details::__ab_to_dq(self, ab, rad);
        return self;
    }
    constexpr AbVoltage to_alpha_beta(const q16 rad) const {
        AbVoltage ab;
        details::__dq_to_ab(ab, *this, rad);
        return ab;
    }
};

}
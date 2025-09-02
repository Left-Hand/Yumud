#pragma once

#include "uvw.hpp"
#include "core/utils/angle.hpp"


namespace ymd::digipw{


namespace details{
template<typename To, typename From>
static constexpr __fast_inline void ab_to_dq(To & dq, const From & ab, const Angle<auto> angle){
    auto [s,c] = angle.sincos();
    dq = {s * ab[1] + c * ab[0], c * ab[1] - s * ab[0]};
};

template<typename To, typename From>
static constexpr __fast_inline void dq_to_ab(To & ab, const From & dq, const Angle<auto> angle){
    auto [s,c] = angle.sincos();
    ab = {c * dq[0] - s * dq[1], c * dq[1] + s * dq[0]};
};
}

struct AlphaBeta final{
    q20 alpha = {};
    q20 beta = {};


    static constexpr AlphaBeta from_uvw(const UvwCurrent & uvw){
        constexpr auto _2_by_3 = q20(2.0/3);
        constexpr auto _sqrt3_by_3 = q20(sqrt(q20(3)) / 3);
        return {(uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, (uvw.v - uvw.w) * _sqrt3_by_3};
    };

    constexpr q20 operator [](const size_t idx) const {
        return *(&alpha + idx);
    }

    constexpr q20 & operator [](const size_t idx){
        return *(&alpha + idx);
    }

    constexpr q20 length() const {
        return sqrt(alpha*alpha + beta*beta);
    }

    friend OutputStream & operator << (OutputStream & os, const AlphaBeta & self){
        return os << os.brackets<'('>() << 
            self.alpha << os.splitter() << 
            self.beta << os.brackets<')'>();
    }
};


struct Dq final{

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


    static constexpr Dq from_alpha_beta(const AlphaBeta & ab, const Angle<auto> angle){
        Dq self;
        details::ab_to_dq(self, ab, angle);
        return self;
    }

    template<typename T>
    constexpr AlphaBeta to_alpha_beta(const Angle<T> angle) const {
        auto [s,c] = angle.sincos();
        auto & self = *this;
        return {c * self.d - s * self.q, c * self.q + s * self.d};
    }

    friend OutputStream & operator << (OutputStream & os, const Dq & self){
        return os << os.brackets<'('>() << 
            self.d << os.splitter() << 
            self.q << os.brackets<')'>();
    }


};

}
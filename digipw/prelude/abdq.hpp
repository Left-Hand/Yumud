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

template<typename T>
struct AlphaBetaCoord final{
    T alpha;
    T beta;


    static constexpr AlphaBetaCoord from_uvw(const UvwCoord & uvw){
        constexpr auto _2_by_3 = T(2.0/3);
        constexpr auto _sqrt3_by_3 = T(sqrt(T(3)) / 3);
        return {(uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, (uvw.v - uvw.w) * _sqrt3_by_3};
    };

    constexpr T operator [](const size_t idx) const {
        return *(&alpha + idx);
    }

    constexpr T & operator [](const size_t idx){
        return *(&alpha + idx);
    }

    constexpr T length() const {
        return sqrt(square(alpha) + square(beta));
    }

    friend OutputStream & operator << (OutputStream & os, const AlphaBetaCoord & self){
        return os << os.brackets<'('>() << 
            self.alpha << os.splitter() << 
            self.beta << os.brackets<')'>();
    }
};


template<typename T>
struct DqCoord final{

    T d;
    T q;

    constexpr T operator [](const size_t idx) const {
        return *(&d + idx);
    }

    constexpr T & operator [](const size_t idx){
        return *(&d + idx);
    }

    constexpr T length() const {
        return mag(d,q);
    }


    static constexpr DqCoord from_alpha_beta(const AlphaBetaCoord<T> & ab, const Angle<auto> angle){
        DqCoord self;
        details::ab_to_dq(self, ab, angle);
        return self;
    }

    template<typename U>
    constexpr AlphaBetaCoord<T> to_alpha_beta(const Angle<U> angle) const {
        auto [s,c] = angle.sincos();
        auto & self = *this;
        return {c * self.d - s * self.q, c * self.q + s * self.d};
    }

    friend OutputStream & operator << (OutputStream & os, const DqCoord & self){
        return os << os.brackets<'('>() << 
            self.d << os.splitter() << 
            self.q << os.brackets<')'>();
    }


};

}
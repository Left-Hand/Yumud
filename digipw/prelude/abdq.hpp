#pragma once

#include "uvw.hpp"
#include "core/utils/angle.hpp"


namespace ymd::digipw{

template<typename T>
struct DqCoord;

namespace details{
template<typename To, typename From>
__attribute__((optimize("O3")))
static constexpr __fast_inline void ab_to_dq(To & dq, const From & ab, const Angle<auto> angle){
    auto [s,c] = angle.sincos();
    dq = {s * ab[1] + c * ab[0], c * ab[1] - s * ab[0]};
};


template<typename To, typename From>
__attribute__((optimize("O3")))
static constexpr __fast_inline void dq_to_ab(To & ab, const From & dq, const Angle<auto> angle){
    auto [s,c] = angle.sincos();
    ab = {c * dq[0] - s * dq[1], c * dq[1] + s * dq[0]};
};
}

template<typename T>
struct AlphaBetaCoord final{
    T alpha;
    T beta;

    static constexpr AlphaBetaCoord ZERO = AlphaBetaCoord{
        static_cast<T>(0),
        static_cast<T>(0)
    };

    [[nodiscard]] static constexpr AlphaBetaCoord from_uvw(const UvwCoord<T> & uvw){
        constexpr auto _2_by_3 = T(2.0/3);
        constexpr auto _sqrt3_by_3 = T(sqrt(T(3)) / 3);
        return {(uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, (uvw.v - uvw.w) * _sqrt3_by_3};
    };

    [[nodiscard]] constexpr T operator [](const size_t idx) const {
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T length() const {
        return sqrt(square(alpha) + square(beta));
    }

    [[nodiscard]] constexpr T length_squared() const {
        return square(alpha) + square(beta);
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator -() const {
        return {-alpha, -beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator +(const AlphaBetaCoord & rhs) const {
        return {alpha + rhs.alpha, beta + rhs.beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator -(const AlphaBetaCoord & rhs) const {
        return {alpha - rhs.alpha, beta - rhs.beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator *(const auto rhs) const {
        return {alpha * rhs, beta * rhs};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator /(const auto rhs) const {
        return {alpha / rhs, beta / rhs};
    }

    [[nodiscard]] constexpr DqCoord<T> to_dq(const Angle<auto> angle) const{
        DqCoord<T> dq;
        details::ab_to_dq(dq, *this, angle);
        return dq;
    }

    [[nodiscard]] constexpr AlphaBetaCoord clamp(const auto max) const {
        return AlphaBetaCoord{
            CLAMP2(this->alpha, max), 
            CLAMP2(this->beta, max)
        };
    }

    template<typename U>
    [[nodiscard]] constexpr operator AlphaBetaCoord<U>(){
        return AlphaBetaCoord<U>{
            static_cast<U>(alpha),
            static_cast<U>(beta)
        };
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

    
    static constexpr DqCoord ZERO = DqCoord{
        static_cast<T>(0),
        static_cast<T>(0)
    };

    [[nodiscard]] constexpr T operator [](const size_t idx) const {
        return *(&d + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&d + idx);
    }

    [[nodiscard]] constexpr T length() const {
        return mag(d,q);
    }

    [[nodiscard]] constexpr DqCoord operator -() const {
        return {-d, -q};
    }

    [[nodiscard]] constexpr DqCoord operator +(const DqCoord & rhs) const {
        return {d + rhs.d, q + rhs.q};
    }

    [[nodiscard]] constexpr DqCoord operator -(const DqCoord & rhs) const {
        return {d - rhs.d, q - rhs.q};
    }

    [[nodiscard]] constexpr DqCoord operator *(const auto rhs) const {
        return {d * rhs, q * rhs};
    }

    [[nodiscard]] constexpr DqCoord operator /(const auto rhs) const {
        return {d / rhs, q / rhs};
    }

    [[nodiscard]] constexpr DqCoord clamp(const auto max) const {
        return DqCoord{
            CLAMP2(this->d, max), 
            CLAMP2(this->q, max)
        };
    }


    [[nodiscard]] static constexpr DqCoord from_alpha_beta(const AlphaBetaCoord<T> & ab, const Angle<auto> angle){
        DqCoord self;
        details::ab_to_dq(self, ab, angle);
        return self;
    }

    template<typename U>
    [[nodiscard]] constexpr AlphaBetaCoord<T> to_alpha_beta(const Angle<U> angle) const {
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
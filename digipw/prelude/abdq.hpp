#pragma once

#include "uvw.hpp"
#include "primitive/arithmetic/angle.hpp"
#include "types/gesture/rotation2.hpp"

namespace ymd::digipw{

template<typename T>
struct DqCoord;

namespace details{
template<typename To, typename From>
__attribute__((optimize("O3")))
static constexpr __fast_inline void alphabeta_to_dq(
    To & dq, const From & alphabeta, const auto & angle
){
    auto [s,c] = angle.sincos();
    dq.template get<0>() = alphabeta.template get<1>() * s + alphabeta.template get<0>() * c;
    dq.template get<1>() = alphabeta.template get<1>() * c - alphabeta.template get<0>() * s;
};


template<typename To, typename From>
__attribute__((optimize("O3")))
static constexpr __fast_inline void dq_to_alphabeta(
    To & alphabeta, const From & dq, const auto & angle
){
    auto [s,c] = angle.sincos();
    alphabeta.template get<0>() = dq.template get<0>() * c - dq.template get<1>() * s;
    alphabeta.template get<1>() = dq.template get<1>() * c + dq.template get<0>() * s;
};
}

template<typename T>
struct [[nodiscard]] AlphaBetaCoord final{
    T alpha;
    T beta;

    static constexpr AlphaBetaCoord ZERO = AlphaBetaCoord{
        static_cast<T>(0),
        static_cast<T>(0)
    };

    enum class Axis { Alpha, Beta };

    [[nodiscard]] static constexpr AlphaBetaCoord from_uvw(const UvwCoord<T> & uvw){
        return AlphaBetaCoord{
            .alpha = (uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, 
            .beta = (uvw.v - uvw.w) * _sqrt3_by_3
        };
    };

    [[nodiscard]] constexpr UvwCoord<T> to_uvw() const{
        return UvwCoord<T>{
            .u = alpha,
            .v = ((beta * _sqrt3_by_2) - (alpha >> 1))
            .w = ((-beta * _sqrt3_by_2) - (alpha >> 1))
        };
    }

    [[nodiscard]] constexpr T operator [](const size_t idx) const {
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T length() const {
        return mag(alpha, beta);
    }

    [[nodiscard]] constexpr T inv_length() const {
        return inv_mag(alpha, beta);
    }

    [[nodiscard]] constexpr T length_squared() const {
        return square(alpha) + square(beta);
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator +() const {
        return AlphaBetaCoord{alpha, beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator -() const {
        return AlphaBetaCoord{-alpha, -beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator +(const AlphaBetaCoord & rhs) const {
        return AlphaBetaCoord{alpha + rhs.alpha, beta + rhs.beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator -(const AlphaBetaCoord & rhs) const {
        return AlphaBetaCoord{alpha - rhs.alpha, beta - rhs.beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator *(const auto rhs) const {
        return AlphaBetaCoord{alpha * rhs, beta * rhs};
    }

    [[nodiscard]] constexpr friend AlphaBetaCoord operator *(const auto lhs, const AlphaBetaCoord & rhs){
        return AlphaBetaCoord{lhs * rhs.alpha, lhs * rhs.beta};
    }

    [[nodiscard]] constexpr Angle<T> angle() const {
        return Angle<T>::from_turns(atan2pu(beta, alpha));
    }

    template<typename Fn>
    [[nodiscard]] constexpr AlphaBetaCoord map(Fn && fn) const {
        return AlphaBetaCoord{
            std::forward<Fn>(fn)(alpha), 
            std::forward<Fn>(fn)(beta)};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator /(const auto rhs) const {
        return AlphaBetaCoord{alpha / rhs, beta / rhs};
    }

    template<typename U>
    [[nodiscard]] constexpr DqCoord<T> to_dq(const Rotation2<U> rot) const{
        DqCoord<T> dq;
        details::alphabeta_to_dq(dq, *this, rot);
        return dq;
    }

    [[nodiscard]] constexpr AlphaBetaCoord clamp(const auto max) const {
        return AlphaBetaCoord{
            CLAMP2(this->alpha, static_cast<T>(max)), 
            CLAMP2(this->beta, static_cast<T>(max))
        };
    }

    template<size_t I>
    requires (I < 2)
    [[nodiscard]] constexpr T get() const{
        if constexpr(I == 0){
            return this->alpha;
        } else if constexpr(I == 1){
            return this->beta;
        }
    } 

    template<size_t I>
    requires (I < 2)
    [[nodiscard]] constexpr T & get(){
        if constexpr(I == 0){
            return this->alpha;
        } else if constexpr(I == 1){
            return this->beta;
        }
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

private:
    static constexpr T _2_by_3 = static_cast<T>(2.0/3);
    static constexpr T _sqrt3_by_3 = static_cast<T>(sqrt(T(3)) / 3);
    static constexpr T _sqrt3_by_2 = static_cast<T>(sqrt(T(3)) / 2);
};

template<typename T>
struct [[nodiscard]] AlphaBetaZeroCoord final{
    T alpha;
    T beta;
    T zero;

    static constexpr AlphaBetaZeroCoord ZERO = AlphaBetaZeroCoord{
        static_cast<T>(0),
        static_cast<T>(0),
        static_cast<T>(0)
    };

    [[nodiscard]] static constexpr AlphaBetaZeroCoord from_uvw(const UvwCoord<T> & uvw){
        return AlphaBetaCoord{
            .alpha = (uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, 
            .beta = (uvw.v - uvw.w) * _sqrt3_by_3,
            .zero = (uvw.u + uvw.v + uvw.w) * _sqrt2_by_2
        };
    };

    [[nodiscard]] constexpr UvwCoord<T> to_uvw() const{
        const auto zero_sqrt2_by_2 = zero * _sqrt2_by_2;
        return UvwCoord<T>{
            .u = alpha + zero_sqrt2_by_2,
            .v = ((beta * _sqrt3_by_2) + zero_sqrt2_by_2 - (alpha >> 1)),
            .w = ((-beta * _sqrt3_by_2) + zero_sqrt2_by_2 - (alpha >> 1))
        };
    }

private:
    static constexpr T _2_by_3 = static_cast<T>(2.0/3);
    static constexpr T _sqrt3_by_3 = static_cast<T>(sqrt(T(3)) / 3);
    static constexpr T _sqrt3_by_2 = static_cast<T>(sqrt(T(3)) / 2);
    static constexpr T _sqrt2_by_2 = static_cast<T>(sqrt(T(2)) / 2);
};


template<typename T>
struct [[nodiscard]] DqCoord final{

    T d;
    T q;

    enum class Axis {D, Q};
    
    static constexpr DqCoord ZERO = DqCoord{
        static_cast<T>(0),
        static_cast<T>(0)
    };

    template<typename U>
    [[nodiscard]] static constexpr DqCoord from_alphabeta(
        const AlphaBetaCoord<T> & ab, 
        const Rotation2<U> rot
    ){
        DqCoord self;
        details::alphabeta_to_dq(self, ab, rot);
        return self;
    }


    [[nodiscard]] constexpr T operator [](const size_t idx) const {
        return *(&d + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&d + idx);
    }

    [[nodiscard]] constexpr T length() const {
        return mag(d,q);
    }

    [[nodiscard]] constexpr DqCoord operator + () const {
        return DqCoord{d, q};
    }

    [[nodiscard]] constexpr DqCoord operator -() const {
        return DqCoord{-d, -q};
    }

    [[nodiscard]] constexpr DqCoord operator +(const DqCoord & rhs) const {
        return DqCoord{d + rhs.d, q + rhs.q};
    }

    [[nodiscard]] constexpr DqCoord operator -(const DqCoord & rhs) const {
        return DqCoord{d - rhs.d, q - rhs.q};
    }

    [[nodiscard]] constexpr DqCoord operator *(const auto rhs) const {
        return DqCoord{d * rhs, q * rhs};
    }

    [[nodiscard]] constexpr DqCoord operator /(const auto rhs) const {
        return DqCoord{d / rhs, q / rhs};
    }

    [[nodiscard]] constexpr DqCoord clamp(const auto max) const {
        return DqCoord{
            CLAMP2(this->d, max), 
            CLAMP2(this->q, max)
        };
    }

    template<size_t I>
    requires (I < 2)
    [[nodiscard]] constexpr T get() const{
        if constexpr(I == 0){
            return this->d;
        } else if constexpr(I == 1){
            return this->q;
        }
    } 

    template<size_t I>
    requires (I < 2)
    [[nodiscard]] constexpr T & get(){
        if constexpr(I == 0){
            return this->d;
        } else if constexpr(I == 1){
            return this->q;
        }
    } 



    template<typename U>
    [[nodiscard]] constexpr AlphaBetaCoord<T> to_alphabeta(const Rotation2<U> rot) const {
        AlphaBetaCoord<T> ret;
        details::dq_to_alphabeta(ret, *this, rot);
        return ret;
    }

    friend OutputStream & operator << (OutputStream & os, const DqCoord & self){
        return os << os.brackets<'('>() << 
            self.d << os.splitter() << 
            self.q << os.brackets<')'>();
    }

};

}

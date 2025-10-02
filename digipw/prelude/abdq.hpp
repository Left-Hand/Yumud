#pragma once

#include "uvw.hpp"
#include "core/utils/angle.hpp"
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
    dq.template get<0>() = s * alphabeta.template get<1>() + c * alphabeta.template get<0>();
    dq.template get<1>() = c * alphabeta.template get<1>() - s * alphabeta.template get<0>();
};


template<typename To, typename From>
__attribute__((optimize("O3")))
static constexpr __fast_inline void dq_to_alphabeta(
    To & alphabeta, const From & dq, const auto & angle
){
    auto [s,c] = angle.sincos();
    alphabeta.template get<0>() = c * dq.template get<0>() - s * dq.template get<1>();
    alphabeta.template get<1>() = c * dq.template get<1>() + s * dq.template get<0>();
};
}

template<typename T>
struct AlphaBetaCoord final{
    T alpha;
    T beta;

    enum class Axis { Alpha, Beta };

    static constexpr AlphaBetaCoord ZERO = AlphaBetaCoord{
        static_cast<T>(0),
        static_cast<T>(0)
    };

    [[nodiscard]] static constexpr AlphaBetaCoord from_uvw(const UvwCoord<T> & uvw){
        return AlphaBetaCoord{
            .alpha = (uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, 
            .beta = (uvw.v - uvw.w) * _sqrt3_by_3
        };
    };

    [[nodiscard]] constexpr T operator [](const size_t idx) const {
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T length() const {
        return imag(alpha + beta);
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
};


template<typename T>
struct DqCoord final{

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

namespace ymd{


template<typename T>
struct FromZeroDispatcher<digipw::AlphaBetaCoord<T>>{
    static consteval digipw::AlphaBetaCoord<T> from_zero() {
        return digipw::AlphaBetaCoord<T>{
            FromZeroDispatcher<T>::from_zero(), 
            FromZeroDispatcher<T>::from_zero()
        };
    }
}; 
template<typename T>
struct FromZeroDispatcher<digipw::DqCoord<T>>{
    static consteval digipw::DqCoord<T> from_zero() {
        return digipw::DqCoord<T>{
            FromZeroDispatcher<T>::from_zero(), 
            FromZeroDispatcher<T>::from_zero()
        };
    }
}; 

}
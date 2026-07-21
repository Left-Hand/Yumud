#pragma once

#include "uvw.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "middlewares/algebra/gesture/rotation2.hpp"

namespace ymd::digipw{

template<typename T>
struct DqCoord;

namespace details{
template<typename To, typename From>
__attribute__((optimize("Ofast"), always_inline))
static constexpr void inv_rotate(
    To & dq, const From & alphabeta, const auto & angle
){
    auto [s,c] = angle.sincos();
    dq.template get<0>() = alphabeta.template get<1>() * s + alphabeta.template get<0>() * c;
    dq.template get<1>() = alphabeta.template get<1>() * c - alphabeta.template get<0>() * s;
};


template<typename To, typename From>
__attribute__((optimize("Ofast"), always_inline))
static constexpr void rotate(
    To & alphabeta, const From & dq, const auto & angle
){
    auto [s,c] = angle.sincos();
    alphabeta.template get<0>() = dq.template get<0>() * c - dq.template get<1>() * s;
    alphabeta.template get<1>() = dq.template get<1>() * c + dq.template get<0>() * s;
};
}

template<typename T>
struct [[nodiscard]] alignas(sizeof(T)) AlphaBetaCoord final{
    static_assert(std::is_signed_v<T>);

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

    [[nodiscard]] constexpr UvwCoord<T> to_uvw() const noexcept {
        const auto half_alpha = (alpha >> 1);
        const auto half_sqrt3_beta = beta * _sqrt3_by_2;
        return UvwCoord<T>{
            .u = alpha,
            .v = ((half_sqrt3_beta) - half_alpha),
            .w = ((-half_sqrt3_beta) - half_alpha)
        };
    }

    [[nodiscard]] constexpr T operator [](const size_t idx) const noexcept {
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&alpha + idx);
    }

    [[nodiscard]] constexpr T length() const noexcept {
        return math::mag(alpha, beta);
    }

    [[nodiscard]] constexpr T inv_length() const noexcept {
        return math::inv_mag(alpha, beta);
    }


    [[nodiscard]] constexpr AlphaBetaCoord operator +() const noexcept {
        return AlphaBetaCoord{alpha, beta};
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator -() const noexcept {
        return AlphaBetaCoord{-alpha, -beta};
    }

    constexpr AlphaBetaCoord & operator +=(const AlphaBetaCoord & rhs) noexcept {
        *this = AlphaBetaCoord{alpha + rhs.alpha, beta + rhs.beta};
        return *this;
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator +(const AlphaBetaCoord & rhs) const noexcept {
        auto ret = *this;
        return (ret += rhs);
    }

    constexpr AlphaBetaCoord & operator -=(const AlphaBetaCoord & rhs) noexcept {
        *this = AlphaBetaCoord{alpha - rhs.alpha, beta - rhs.beta};
        return *this;
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator -(const AlphaBetaCoord & rhs) const noexcept {
        auto ret = *this;
        return (ret -= rhs);
    }

    [[nodiscard]] constexpr AlphaBetaCoord operator *(const auto rhs) const noexcept {
        return AlphaBetaCoord{alpha * rhs, beta * rhs};
    }

    [[nodiscard]] constexpr friend AlphaBetaCoord operator *(const auto lhs, const AlphaBetaCoord & rhs){
        return AlphaBetaCoord{lhs * rhs.alpha, lhs * rhs.beta};
    }


    template<typename Fn>
    [[nodiscard]] constexpr AlphaBetaCoord map(Fn && fn) const noexcept {
        return AlphaBetaCoord{
            std::forward<Fn>(fn)(alpha), 
            std::forward<Fn>(fn)(beta)};
    }


    template<typename U>
    [[nodiscard]] constexpr DqCoord<T> inv_rotate(const math::Rotation2<U> rot) const noexcept {
        DqCoord<T> dq;
        details::inv_rotate(dq, *this, rot);
        return dq;
    }

    template<size_t I>
    requires (I < 2)
    [[nodiscard]] constexpr T get() const noexcept {
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
    static constexpr uq32 _2_by_3 = static_cast<T>(2.0/3);
    static constexpr uq32 _sqrt3_by_3 = static_cast<T>(1.73205080757 / 3);
    static constexpr uq32 _sqrt3_by_2 = static_cast<T>(1.73205080757 / 2);
};

template<typename T>
struct [[nodiscard]] alignas(sizeof(T)) AlphaBetaZeroCoord final{
    static_assert(std::is_signed_v<T>);

    T alpha;
    T beta;
    T zero;

    static constexpr AlphaBetaZeroCoord ZERO = AlphaBetaZeroCoord{
        static_cast<T>(0),
        static_cast<T>(0),
        static_cast<T>(0)
    };

    [[nodiscard]] static constexpr AlphaBetaZeroCoord from_uninitialized(){
        return {};
    }

    [[nodiscard]] static constexpr AlphaBetaZeroCoord from_uvw(const UvwCoord<T> & uvw){
        return AlphaBetaCoord{
            .alpha = (uvw.u - ((uvw.v + uvw.w) >> 1)) * _2_by_3, 
            .beta = (uvw.v - uvw.w) * _sqrt3_by_3,
            .zero = (uvw.u + uvw.v + uvw.w) * _sqrt2_by_2
        };
    };

    [[nodiscard]] constexpr UvwCoord<T> to_uvw() const noexcept {
        const auto zero_sqrt2_by_2 = zero * _sqrt2_by_2;
        return UvwCoord<T>{
            .u = alpha + zero_sqrt2_by_2,
            .v = ((beta * _sqrt3_by_2) + zero_sqrt2_by_2 - (alpha >> 1)),
            .w = ((-beta * _sqrt3_by_2) + zero_sqrt2_by_2 - (alpha >> 1))
        };
    }

private:
    static constexpr uq32 _2_by_3 = static_cast<uq32>(2.0/3);
    static constexpr uq32 _sqrt3_by_3 = static_cast<uq32>(1.73205080757 / 3);
    static constexpr uq32 _sqrt3_by_2 = static_cast<uq32>(1.73205080757 / 2);
    static constexpr uq32 _sqrt2_by_2 = static_cast<uq32>(math::sqrt(T(2)) / 2);
};


template<typename T>
struct [[nodiscard]] alignas(sizeof(T)) DqCoord final{
    static_assert(std::is_signed_v<T>);
    
    T d;
    T q;

    enum class Axis {D, Q};
    
    static constexpr DqCoord ZERO = DqCoord{
        static_cast<T>(0),
        static_cast<T>(0)
    };


    [[nodiscard]] static constexpr DqCoord from_uninitialized(){
        return {};
    }



    [[nodiscard]] constexpr T operator [](const size_t idx) const noexcept {
        return *(&d + idx);
    }

    [[nodiscard]] constexpr T & operator [](const size_t idx){
        return *(&d + idx);
    }

    [[nodiscard]] constexpr T length() const noexcept {
        return math::mag(d,q);
    }

    [[nodiscard]] constexpr DqCoord operator + () const noexcept {
        return DqCoord{d, q};
    }

    [[nodiscard]] constexpr DqCoord operator -() const noexcept {
        return DqCoord{-d, -q};
    }

    [[nodiscard]] constexpr DqCoord operator +(const DqCoord & rhs) const noexcept {
        return DqCoord{d + rhs.d, q + rhs.q};
    }

    [[nodiscard]] constexpr DqCoord operator -(const DqCoord & rhs) const noexcept {
        return DqCoord{d - rhs.d, q - rhs.q};
    }

    [[nodiscard]] constexpr DqCoord operator *(const auto rhs) const noexcept {
        return DqCoord{d * rhs, q * rhs};
    }


    constexpr DqCoord & operator +=(const DqCoord & rhs) noexcept {
        *this = DqCoord{d + rhs.d, q + rhs.q};
        return *this;
    }

    constexpr DqCoord & operator -=(const DqCoord & rhs) const noexcept {
        *this = DqCoord{d - rhs.d, q - rhs.q};
        return *this;
    }



    template<size_t I>
    requires (I < 2)
    [[nodiscard]] constexpr T get() const noexcept {
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
    [[nodiscard]] constexpr AlphaBetaCoord<T> rotate(const U rot) const noexcept {
        AlphaBetaCoord<T> ret;
        details::rotate(ret, *this, rot);
        return ret;
    }

    friend OutputStream & operator << (OutputStream & os, const DqCoord & self){
        return os << os.brackets<'('>() << 
            self.d << os.splitter() << 
            self.q << os.brackets<')'>();
    }

};

}

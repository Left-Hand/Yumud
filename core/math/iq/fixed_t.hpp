#pragma once


#include "core/platform.hpp"

#include "support.hpp"
#include "_IQNdiv.hpp"
#include "_IQNconv.hpp"

#ifdef YUMUD_FRAMEWORK
//引入浮点的结构模型 以方便进行和浮点数转换
#include "core/math/float/fp32.hpp"
#include "core/math/float/fp64.hpp"
#endif

#include <numeric>

#ifndef LOG_E
#define LOG_E (0.434294481903)
#endif

#ifndef IQ_DEFAULT_Q
#define IQ_DEFAULT_Q 16
#endif



namespace ymd{
template<size_t Q, typename D>
struct fixed_t;
}

namespace std{
    template<size_t Q, typename D>
    struct is_arithmetic<ymd::fixed_t<Q, D>> : std::true_type {};

    template<size_t Q, typename D>
    struct is_floating_point<ymd::fixed_t<Q, D>> : std::false_type {};

    template<size_t Q, typename D>
    struct is_signed<ymd::fixed_t<Q, D>> : std::is_signed<D> {};


}

namespace ymd{
// 默认模板：非定点数类型
template<typename T>
constexpr bool is_fixed_point_v = false;

// 特化模板：定点数类型
template<size_t Q, typename D>
constexpr bool is_fixed_point_v<fixed_t<Q, D>> = true;

namespace iqmath::details{
template<typename U>
struct upcast_underlying_type;

template<>
struct upcast_underlying_type<uint8_t>{
    using type = uint16_t;
};

template<>
struct upcast_underlying_type<uint16_t>{
    using type = uint32_t;
};

template<>
struct upcast_underlying_type<uint32_t>{
    using type = uint64_t;
};

template<>
struct upcast_underlying_type<int8_t>{
    using type = int16_t;
};

template<>
struct upcast_underlying_type<int16_t>{
    using type = int32_t;
};

template<>
struct upcast_underlying_type<int32_t>{
    using type = int64_t;
};


}



template<size_t Q, typename D>
struct fixed_t{
private:
    static_assert(Q <= sizeof(D) * 8);

    D count_;

    
    using upcast_underlying_t = typename iqmath::details::upcast_underlying_type<D>::type;

public:
    struct count_ctor{
        D count;
    };

    template<size_t P>
    [[nodiscard]] __fast_inline static constexpr D transform(const D value){
        if constexpr (P > Q){
            return D(static_cast<upcast_underlying_t>(value) << (P - Q));
        }else if constexpr (P < Q){
            return D(static_cast<upcast_underlying_t>(value) >> (Q - P));
        }else{
            return value;
        }
    }

    static constexpr size_t q_num = Q;

    __fast_inline constexpr fixed_t(){;}

    __fast_inline constexpr fixed_t(count_ctor ctor):
        count_(ctor.count){;}


    
    __fast_inline constexpr fixed_t(const fixed_t<Q, D> & other):
        count_(other.count_){};


    template<size_t P, typename U>
    requires (sizeof(D) == sizeof(U))
    __fast_inline constexpr operator fixed_t<P, U>() const {
        const auto bits = static_cast<U>(transform<P>(count_));
        return fixed_t<P, U>::from_bits(bits);
    }

    __fast_inline constexpr D as_bits() const {return count_;}
    
    template<size_t P>
    __fast_inline constexpr fixed_t & operator = (const fixed_t<P, D> & other){
        count_ = fixed_t<Q, D>::template transform<Q>(other.as_bits());
        return *this;
    };

    template<size_t P>
    __fast_inline constexpr fixed_t & operator = (fixed_t<P, D> && other){
        count_ = fixed_t<Q, D>::template transform<Q>(other.as_bits());
        return *this;
    };
    
    static constexpr fixed_t<Q, D> from_bits(const D count){
        return fixed_t<Q, D>(count_ctor{count});
    };

    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr fixed_t(const T iv):
        fixed_t(count_ctor{static_cast<D>(iv * static_cast<upcast_underlying_t>(uint64_t(1) << Q))}){;}

    __fast_inline consteval explicit fixed_t(const long double dv):
        fixed_t(count_ctor{static_cast<D>(dv * static_cast<long double>(uint64_t(1) << Q))}){};

    static __fast_inline constexpr fixed_t from (const floating auto fv){
        return fixed_t{count_ctor{
            .count = static_cast<D>(iqmath::details::_IQFtoN<Q>(fv))}
        };
    }

    __fast_inline constexpr fixed_t operator+() const {
        return *this;
    }

    __fast_inline constexpr fixed_t operator-() const {
        return fixed_t::from_bits(-(as_bits()));
    }

    //#region addsub
    template<size_t P>
    __fast_inline constexpr fixed_t & operator +=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(this->as_bits() + fixed_t<Q, D>(other).as_bits());
    }

    __fast_inline constexpr fixed_t & operator +=(const integral auto other) {
        return *this += fixed_t<Q, D>(other);
    }


    template<size_t P>
    __fast_inline constexpr fixed_t & operator -=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(this->as_bits() - fixed_t<Q, D>(other).as_bits());
    }

    __fast_inline constexpr fixed_t & operator -=(const integral auto other) {
        return *this -= fixed_t<Q, D>(other);
    }
    //#endregion addsub

    //#region multiply

    template<size_t P>
    __fast_inline constexpr fixed_t& operator *=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(
            (static_cast<upcast_underlying_t>(this->as_bits()) * static_cast<upcast_underlying_t>((other).as_bits())) >> (P)
        );
    }

    __fast_inline constexpr fixed_t& operator *=(const integral auto other) {
        return *this = fixed_t<Q, D>::from_bits(this->as_bits() * other);
    }
    
    //#endregion

    //#region division
    __fast_inline constexpr fixed_t & operator/=(const integral auto other) {
        return *this = fixed_t::from_bits((as_bits() / D(other)));
    }

    template<size_t P>
    __fast_inline constexpr fixed_t & operator/=(const fixed_t<P, D> other) {
        if (std::is_constant_evaluated()) {
            return *this = fixed_t<Q, D>::from(float(*this) / float(other));
        }else{
            return *this = fixed_t::from_bits(iqmath::details::__IQNdiv_impl<Q, false>(
                as_bits(), other.as_bits()
            ));
        }
    }

    //#endregion

    //#region comparisons
    #define IQ_COMP_TEMPLATE(op)\
    template<size_t P>\
    [[nodiscard]] __fast_inline constexpr bool operator op (const fixed_t<P, D> other) const {\
        if constexpr(P == Q){\
            return as_bits() op (other.as_bits());\
        }else{\
            return (static_cast<upcast_underlying_t>(as_bits()) << P) op (static_cast<upcast_underlying_t>(other.as_bits()) << Q);\
        }\
    }\
    \
    template<typename T>\
    requires std::is_floating_point_v<T>\
    [[nodiscard]] __fast_inline constexpr bool operator op (const T other) const {\
        return (*this op fixed_t<Q, D>(other));\
    }\
    \
    template<typename T>\
    requires std::is_integral_v<T>\
    [[nodiscard]] __fast_inline constexpr bool operator op (const T other) const {\
        return (((as_bits())) op (D(other) << Q));\
    }\
    template<typename T>\
    requires std::is_integral_v<T>\
    [[nodiscard]] __fast_inline friend constexpr bool operator op (const T other, const fixed_t & self){\
        return (((self.as_bits())) op (D(other) << Q));\
    }\


    IQ_COMP_TEMPLATE(==)
    IQ_COMP_TEMPLATE(!=)
    IQ_COMP_TEMPLATE(>)
    IQ_COMP_TEMPLATE(<)
    IQ_COMP_TEMPLATE(>=)
    IQ_COMP_TEMPLATE(<=)
    #undef IQ_COMP_TEMPLATE
    //#endregion

    //#region shifts
    [[nodiscard]] __fast_inline constexpr fixed_t operator<<(size_t shift) const {
        return fixed_t::from_bits((this->as_bits() << shift));
    }

    [[nodiscard]] __fast_inline constexpr fixed_t operator>>(size_t shift) const {
        return fixed_t::from_bits((this->as_bits() >> shift));
    }
    //#endregion

    [[nodiscard]] __fast_inline constexpr explicit operator bool() const {
        return bool(this->as_bits());
    }

    template<typename T>
    requires std::is_integral_v<T>
    [[nodiscard]] __fast_inline constexpr explicit operator T() const {
        return this->as_bits() >> Q;
    }
    

    template<typename T>
    requires std::is_floating_point_v<T>
    __inline constexpr explicit operator T() const{
        if(std::is_constant_evaluated()){
            return float(this->as_bits()) / D(1u << Q);
        }else{
            return iqmath::details::_IQNtoF<Q>(this->as_bits());
        }
    }
};

template<size_t Q, size_t P, typename D>
__fast_inline constexpr fixed_t<Q, D> operator +(const fixed_t<Q, D> lhs, const fixed_t<P, D> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(rhs);
    ret += lhs;
    return ret;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator +(const integral auto lhs, const fixed_t<Q, D> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret += rhs;
    return ret;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator +(const fixed_t<Q, D> lhs, const integral auto rhs) {
    fixed_t<Q, D> ret = lhs;
    ret += fixed_t<Q, D>(rhs);
    return ret;
}

template<size_t Q, size_t P, typename D>
__fast_inline constexpr fixed_t<Q, D> operator -(const fixed_t<Q, D> lhs, const fixed_t<P, D> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret -= rhs;
    return ret;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator -(const integral auto lhs, const fixed_t<Q, D> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret -= rhs;
    return ret;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator -(const fixed_t<Q, D> lhs, const integral auto rhs) {
    fixed_t<Q, D> ret = lhs;
    ret -= fixed_t<Q, D>(rhs);
    return ret;
}


template<size_t Q, size_t P, typename D>
__fast_inline constexpr fixed_t<Q, D> operator *(const fixed_t<Q, D> lhs, const fixed_t<P, D> rhs) {
    fixed_t<Q, D> ret = lhs;
    ret *= rhs;
    return ret;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator *(const integral auto lhs, const fixed_t<Q, D> rhs) {
    fixed_t<Q, D> ret = rhs;
    ret *= lhs;
    return ret;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator *(const fixed_t<Q, D> lhs, const integral auto rhs) {
    fixed_t<Q, D> ret = lhs;
    ret *= rhs;
    return ret;
}

template<size_t Q, size_t P, typename D>
__fast_inline constexpr fixed_t<Q, D> operator /(const fixed_t<Q, D> lhs, const fixed_t<P, D> rhs) {
    fixed_t<Q, D> ret = lhs;
    ret /= rhs;
    return ret;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator /(const integral auto lhs, const fixed_t<Q, D> rhs) {
	fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret /= rhs;
    return ret;
}


template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> operator /(const fixed_t<Q, D> lhs, const integral auto rhs) {
	fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret /= rhs;
    return ret;
}

// template<typename T, size_t Q, typename D>
// requires (std::is_convertible_v<T, fixed_t<Q, D>> && (!is_fixed_point_v<T>))
// __fast_inline constexpr auto operator <=>(const T lhs, const fixed_t<Q, D> rhs) {
// 	return fixed_t<Q, D>(lhs) <=> rhs;
// }



template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> abs(const fixed_t<Q, D> x){
    const auto ivalue = x.as_bits();
    return fixed_t<Q, D>::from_bits(ivalue > 0 ? ivalue : -ivalue);
}

template<size_t Q, typename D>
__fast_inline constexpr bool signbit(const fixed_t<Q, D> x){
    constexpr D SIGN_MASK = static_cast<D>(static_cast<D>(1) << size_t(sizeof(D) * 8 - 1));
    return static_cast<bool>(x.as_bits() & SIGN_MASK);
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> sign(const fixed_t<Q, D> x){
    if(likely(x)) return fixed_t<Q, D>(x > 0 ? 1 : -1);
    else return fixed_t<Q, D>(0);
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> fmod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits(a.as_bits() % b.as_bits()));
}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
__fast_inline constexpr fixed_t<Q, U> fposmod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    constexpr size_t SHIFT = size_t(sizeof(D) * 8 - 1);
    const U mod_result = std::bit_cast<U>(a.as_bits() % b.as_bits());
    const U is_negative = static_cast<U>(mod_result >> SHIFT);  // 符号位扩展（0 或 -1）
    return fixed_t<Q, U>::from_bits(static_cast<U>(mod_result + static_cast<U>(b.as_bits() & is_negative)));
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> lerp(const fixed_t<Q, D> x, const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return a * (1 - x) + b * x;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> mean(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits((a.as_bits() + b.as_bits()) >> 1));}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
__fast_inline constexpr fixed_t<Q, U> frac(const fixed_t<Q, D> x){
    static constexpr U MASK = static_cast<U>((uint64_t(1u) << Q) - 1);
    return fixed_t<Q, U>::from_bits(static_cast<U>(static_cast<U>(x.as_bits()) & MASK));
}


template<size_t Q, typename D>
__fast_inline constexpr D floor_int(const fixed_t<Q, D> x){
    return static_cast<D>(x.as_bits() >> Q);}

template<size_t Q, typename D>
__fast_inline constexpr D ceil_int(const fixed_t<Q, D> x){
    static constexpr D MASK = (1 << Q) - 1;
    return static_cast<D>((x.as_bits() >> Q) + bool(x.as_bits() & MASK));
}

template<size_t Q, typename D>
__fast_inline constexpr D round_int(const fixed_t<Q, D> x){
    static constexpr D MASK = (1 << (Q - 1));
    return static_cast<D>((x.as_bits() + MASK) >> Q);
}

template<typename T, size_t Q, typename D>
__fast_inline constexpr T floor_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(floor_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<typename T, size_t Q, typename D>
__fast_inline constexpr T ceil_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(ceil_int(x));
    }else{
        return static_cast<T>(x);
    }
}
template<typename T, size_t Q, typename D>
__fast_inline constexpr T round_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(round_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> floor(const fixed_t<Q, D> x){
    return floor_int(x);
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> ceil(const fixed_t<Q, D> x){
    return ceil_int(x);
}



template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> round(const fixed_t<Q, D> x){
    return round_int(x);
}



template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> square(const fixed_t<Q, D> x) {
    return x * x;
}


template<size_t Q, typename D>
static __fast_inline constexpr fixed_t<Q, D> errmod(const fixed_t<Q, D> x, const fixed_t<Q, D> s) {
    const auto s_by_2 = s >> 1;
    fixed_t<Q, D> value = fmod(x, s);
    if (value > s_by_2) {
        value -= s;
    } else if (value <= -s_by_2) {
        value += s;
    }
    return value;
}

}

namespace std{
    using ymd::fixed_t;
    template<size_t Q, typename D>
    class numeric_limits<fixed_t<Q, D>> {
    public:
        __fast_inline constexpr static fixed_t<Q, D> infinity() noexcept {
            return fixed_t<Q, D>::from_bits(std::numeric_limits<D>::infinity());}
        __fast_inline constexpr static fixed_t<Q, D> lowest() noexcept {
            return fixed_t<Q, D>::from_bits(std::numeric_limits<D>::lowest());}

        __fast_inline constexpr static fixed_t<Q, D> min() noexcept {
            return fixed_t<Q, D>::from_bits(std::numeric_limits<D>::min());}
        __fast_inline constexpr static fixed_t<Q, D> max() noexcept {
            return fixed_t<Q, D>::from_bits(std::numeric_limits<D>::max());}
    };

    template<size_t Q, typename D>
    struct common_type<fixed_t<Q, D>, float> {
        using type = fixed_t<Q, D>;
    };

    template<size_t Q, typename D>
    struct common_type<fixed_t<Q, D>, double> {
        using type = fixed_t<Q, D>;
    };

    template<size_t Q, typename D>
    struct common_type<float, fixed_t<Q, D>> {
        using type = fixed_t<Q, D>;
    };

    template<size_t Q, typename D>
    struct common_type<double, fixed_t<Q, D>> {
        using type = fixed_t<Q, D>;
    };

    template<size_t Q, typename D>
    __fast_inline constexpr auto signbit(const fixed_t<Q, D> iq)  {return ymd::signbit(iq);}
    
    template<size_t Q, typename D>
    struct make_signed<fixed_t<Q, D>>{
        using type = fixed_t<Q, D>;
    };

    template<size_t Q, typename D>
    struct make_unsigned<fixed_t<Q, D>>{
        using type = fixed_t<Q, D>;
    };
}


namespace ymd::literals{ 
using iq1 = fixed_t<1, int32_t>;
using iq2 = fixed_t<2, int32_t>;
using iq3 = fixed_t<3, int32_t>;
using iq4 = fixed_t<4, int32_t>;
using iq5 = fixed_t<5, int32_t>;
using iq6 = fixed_t<6, int32_t>;
using iq7 = fixed_t<7, int32_t>;
using iq8 = fixed_t<8, int32_t>;
using iq9 = fixed_t<9, int32_t>;
using iq10 = fixed_t<10, int32_t>;
using iq11 = fixed_t<11, int32_t>;
using iq12 = fixed_t<12, int32_t>;
using iq13 = fixed_t<13, int32_t>;
using iq14 = fixed_t<14, int32_t>;
using iq15 = fixed_t<15, int32_t>;
using iq16 = fixed_t<16, int32_t>;
using iq17 = fixed_t<17, int32_t>;
using iq18 = fixed_t<18, int32_t>;
using iq19 = fixed_t<19, int32_t>;
using iq20 = fixed_t<20, int32_t>;
using iq21 = fixed_t<21, int32_t>;
using iq22 = fixed_t<22, int32_t>;
using iq23 = fixed_t<23, int32_t>;
using iq24 = fixed_t<24, int32_t>;
using iq25 = fixed_t<25, int32_t>;
using iq26 = fixed_t<26, int32_t>;
using iq27 = fixed_t<27, int32_t>;
using iq28 = fixed_t<28, int32_t>;
using iq29 = fixed_t<29, int32_t>;
using iq30 = fixed_t<30, int32_t>;
using iq31 = fixed_t<31, int32_t>;

using iiq1 = fixed_t<1, int64_t>;
using iiq2 = fixed_t<2, int64_t>;
using iiq3 = fixed_t<3, int64_t>;
using iiq4 = fixed_t<4, int64_t>;
using iiq5 = fixed_t<5, int64_t>;
using iiq6 = fixed_t<6, int64_t>;
using iiq7 = fixed_t<7, int64_t>;
using iiq8 = fixed_t<8, int64_t>;
using iiq9 = fixed_t<9, int64_t>;
using iiq10 = fixed_t<10, int64_t>;
using iiq11 = fixed_t<11, int64_t>;
using iiq12 = fixed_t<12, int64_t>;
using iiq13 = fixed_t<13, int64_t>;
using iiq14 = fixed_t<14, int64_t>;
using iiq15 = fixed_t<15, int64_t>;
using iiq16 = fixed_t<16, int64_t>;
using iiq17 = fixed_t<17, int64_t>;
using iiq18 = fixed_t<18, int64_t>;
using iiq19 = fixed_t<19, int64_t>;
using iiq20 = fixed_t<20, int64_t>;
using iiq21 = fixed_t<21, int64_t>;
using iiq22 = fixed_t<22, int64_t>;
using iiq23 = fixed_t<23, int64_t>;
using iiq24 = fixed_t<24, int64_t>;
using iiq25 = fixed_t<25, int64_t>;
using iiq26 = fixed_t<26, int64_t>;
using iiq27 = fixed_t<27, int64_t>;
using iiq28 = fixed_t<28, int64_t>;
using iiq29 = fixed_t<29, int64_t>;
using iiq30 = fixed_t<30, int64_t>;
using iiq31 = fixed_t<31, int64_t>;

using uq1 = fixed_t<1, uint32_t>;
using uq2 = fixed_t<2, uint32_t>;
using uq3 = fixed_t<3, uint32_t>;
using uq4 = fixed_t<4, uint32_t>;
using uq5 = fixed_t<5, uint32_t>;
using uq6 = fixed_t<6, uint32_t>;
using uq7 = fixed_t<7, uint32_t>;
using uq8 = fixed_t<8, uint32_t>;
using uq9 = fixed_t<9, uint32_t>;
using uq10 = fixed_t<10, uint32_t>;
using uq11 = fixed_t<11, uint32_t>;
using uq12 = fixed_t<12, uint32_t>;
using uq13 = fixed_t<13, uint32_t>;
using uq14 = fixed_t<14, uint32_t>;
using uq15 = fixed_t<15, uint32_t>;
using uq16 = fixed_t<16, uint32_t>;
using uq17 = fixed_t<17, uint32_t>;
using uq18 = fixed_t<18, uint32_t>;
using uq19 = fixed_t<19, uint32_t>;
using uq20 = fixed_t<20, uint32_t>;
using uq21 = fixed_t<21, uint32_t>;
using uq22 = fixed_t<22, uint32_t>;
using uq23 = fixed_t<23, uint32_t>;
using uq24 = fixed_t<24, uint32_t>;
using uq25 = fixed_t<25, uint32_t>;
using uq26 = fixed_t<26, uint32_t>;
using uq27 = fixed_t<27, uint32_t>;
using uq28 = fixed_t<28, uint32_t>;
using uq29 = fixed_t<29, uint32_t>;
using uq30 = fixed_t<30, uint32_t>;
using uq31 = fixed_t<31, uint32_t>;
using uq32 = fixed_t<32, uint32_t>;

using uuq1 = fixed_t<1, uint64_t>;
using uuq2 = fixed_t<2, uint64_t>;
using uuq3 = fixed_t<3, uint64_t>;
using uuq4 = fixed_t<4, uint64_t>;
using uuq5 = fixed_t<5, uint64_t>;
using uuq6 = fixed_t<6, uint64_t>;
using uuq7 = fixed_t<7, uint64_t>;
using uuq8 = fixed_t<8, uint64_t>;
using uuq9 = fixed_t<9, uint64_t>;
using uuq10 = fixed_t<10, uint64_t>;
using uuq11 = fixed_t<11, uint64_t>;
using uuq12 = fixed_t<12, uint64_t>;
using uuq13 = fixed_t<13, uint64_t>;
using uuq14 = fixed_t<14, uint64_t>;
using uuq15 = fixed_t<15, uint64_t>;
using uuq16 = fixed_t<16, uint64_t>;
using uuq17 = fixed_t<17, uint64_t>;
using uuq18 = fixed_t<18, uint64_t>;
using uuq19 = fixed_t<19, uint64_t>;
using uuq20 = fixed_t<20, uint64_t>;
using uuq21 = fixed_t<21, uint64_t>;
using uuq22 = fixed_t<22, uint64_t>;
using uuq23 = fixed_t<23, uint64_t>;
using uuq24 = fixed_t<24, uint64_t>;
using uuq25 = fixed_t<25, uint64_t>;
using uuq26 = fixed_t<26, uint64_t>;
using uuq27 = fixed_t<27, uint64_t>;
using uuq28 = fixed_t<28, uint64_t>;
using uuq29 = fixed_t<29, uint64_t>;
using uuq30 = fixed_t<30, uint64_t>;
using uuq31 = fixed_t<31, uint64_t>;

consteval fixed_t<1, int32_t> operator"" _iq1(long double x) { return fixed_t<1, int32_t>(x); }
consteval fixed_t<2, int32_t> operator"" _iq2(long double x) { return fixed_t<2, int32_t>(x); }
consteval fixed_t<3, int32_t> operator"" _iq3(long double x) { return fixed_t<3, int32_t>(x); }
consteval fixed_t<4, int32_t> operator"" _iq4(long double x) { return fixed_t<4, int32_t>(x); }
consteval fixed_t<5, int32_t> operator"" _iq5(long double x) { return fixed_t<5, int32_t>(x); }
consteval fixed_t<6, int32_t> operator"" _iq6(long double x) { return fixed_t<6, int32_t>(x); }
consteval fixed_t<7, int32_t> operator"" _iq7(long double x) { return fixed_t<7, int32_t>(x); }
consteval fixed_t<8, int32_t> operator"" _iq8(long double x) { return fixed_t<8, int32_t>(x); }
consteval fixed_t<9, int32_t> operator"" _iq9(long double x) { return fixed_t<9, int32_t>(x); }
consteval fixed_t<10, int32_t> operator"" _iq10(long double x) { return fixed_t<10, int32_t>(x); }
consteval fixed_t<11, int32_t> operator"" _iq11(long double x) { return fixed_t<11, int32_t>(x); }
consteval fixed_t<12, int32_t> operator"" _iq12(long double x) { return fixed_t<12, int32_t>(x); }
consteval fixed_t<13, int32_t> operator"" _iq13(long double x) { return fixed_t<13, int32_t>(x); }
consteval fixed_t<14, int32_t> operator"" _iq14(long double x) { return fixed_t<14, int32_t>(x); }
consteval fixed_t<15, int32_t> operator"" _iq15(long double x) { return fixed_t<15, int32_t>(x); }
consteval fixed_t<16, int32_t> operator"" _iq16(long double x) { return fixed_t<16, int32_t>(x); }
consteval fixed_t<17, int32_t> operator"" _iq17(long double x) { return fixed_t<17, int32_t>(x); }
consteval fixed_t<18, int32_t> operator"" _iq18(long double x) { return fixed_t<18, int32_t>(x); }
consteval fixed_t<19, int32_t> operator"" _iq19(long double x) { return fixed_t<19, int32_t>(x); }
consteval fixed_t<20, int32_t> operator"" _iq20(long double x) { return fixed_t<20, int32_t>(x); }
consteval fixed_t<21, int32_t> operator"" _iq21(long double x) { return fixed_t<21, int32_t>(x); }
consteval fixed_t<22, int32_t> operator"" _iq22(long double x) { return fixed_t<22, int32_t>(x); }
consteval fixed_t<23, int32_t> operator"" _iq23(long double x) { return fixed_t<23, int32_t>(x); }
consteval fixed_t<24, int32_t> operator"" _iq24(long double x) { return fixed_t<24, int32_t>(x); }
consteval fixed_t<25, int32_t> operator"" _iq25(long double x) { return fixed_t<25, int32_t>(x); }
consteval fixed_t<26, int32_t> operator"" _iq26(long double x) { return fixed_t<26, int32_t>(x); }
consteval fixed_t<27, int32_t> operator"" _iq27(long double x) { return fixed_t<27, int32_t>(x); }
consteval fixed_t<28, int32_t> operator"" _iq28(long double x) { return fixed_t<28, int32_t>(x); }
consteval fixed_t<29, int32_t> operator"" _iq29(long double x) { return fixed_t<29, int32_t>(x); }
consteval fixed_t<30, int32_t> operator"" _iq30(long double x) { return fixed_t<30, int32_t>(x); }
consteval fixed_t<31, int32_t> operator"" _iq31(long double x) { return fixed_t<31, int32_t>(x); }

consteval fixed_t<1, int32_t> operator"" _iq1(unsigned long long x) { return fixed_t<1, int32_t>(x); }
consteval fixed_t<2, int32_t> operator"" _iq2(unsigned long long x) { return fixed_t<2, int32_t>(x); }
consteval fixed_t<3, int32_t> operator"" _iq3(unsigned long long x) { return fixed_t<3, int32_t>(x); }
consteval fixed_t<4, int32_t> operator"" _iq4(unsigned long long x) { return fixed_t<4, int32_t>(x); }
consteval fixed_t<5, int32_t> operator"" _iq5(unsigned long long x) { return fixed_t<5, int32_t>(x); }
consteval fixed_t<6, int32_t> operator"" _iq6(unsigned long long x) { return fixed_t<6, int32_t>(x); }
consteval fixed_t<7, int32_t> operator"" _iq7(unsigned long long x) { return fixed_t<7, int32_t>(x); }
consteval fixed_t<8, int32_t> operator"" _iq8(unsigned long long x) { return fixed_t<8, int32_t>(x); }
consteval fixed_t<9, int32_t> operator"" _iq9(unsigned long long x) { return fixed_t<9, int32_t>(x); }
consteval fixed_t<10, int32_t> operator"" _iq10(unsigned long long x) { return fixed_t<10, int32_t>(x); }
consteval fixed_t<11, int32_t> operator"" _iq11(unsigned long long x) { return fixed_t<11, int32_t>(x); }
consteval fixed_t<12, int32_t> operator"" _iq12(unsigned long long x) { return fixed_t<12, int32_t>(x); }
consteval fixed_t<13, int32_t> operator"" _iq13(unsigned long long x) { return fixed_t<13, int32_t>(x); }
consteval fixed_t<14, int32_t> operator"" _iq14(unsigned long long x) { return fixed_t<14, int32_t>(x); }
consteval fixed_t<15, int32_t> operator"" _iq15(unsigned long long x) { return fixed_t<15, int32_t>(x); }
consteval fixed_t<16, int32_t> operator"" _iq16(unsigned long long x) { return fixed_t<16, int32_t>(x); }
consteval fixed_t<17, int32_t> operator"" _iq17(unsigned long long x) { return fixed_t<17, int32_t>(x); }
consteval fixed_t<18, int32_t> operator"" _iq18(unsigned long long x) { return fixed_t<18, int32_t>(x); }
consteval fixed_t<19, int32_t> operator"" _iq19(unsigned long long x) { return fixed_t<19, int32_t>(x); }
consteval fixed_t<20, int32_t> operator"" _iq20(unsigned long long x) { return fixed_t<20, int32_t>(x); }
consteval fixed_t<21, int32_t> operator"" _iq21(unsigned long long x) { return fixed_t<21, int32_t>(x); }
consteval fixed_t<22, int32_t> operator"" _iq22(unsigned long long x) { return fixed_t<22, int32_t>(x); }
consteval fixed_t<23, int32_t> operator"" _iq23(unsigned long long x) { return fixed_t<23, int32_t>(x); }
consteval fixed_t<24, int32_t> operator"" _iq24(unsigned long long x) { return fixed_t<24, int32_t>(x); }
consteval fixed_t<25, int32_t> operator"" _iq25(unsigned long long x) { return fixed_t<25, int32_t>(x); }
consteval fixed_t<26, int32_t> operator"" _iq26(unsigned long long x) { return fixed_t<26, int32_t>(x); }
consteval fixed_t<27, int32_t> operator"" _iq27(unsigned long long x) { return fixed_t<27, int32_t>(x); }
consteval fixed_t<28, int32_t> operator"" _iq28(unsigned long long x) { return fixed_t<28, int32_t>(x); }
consteval fixed_t<29, int32_t> operator"" _iq29(unsigned long long x) { return fixed_t<29, int32_t>(x); }
consteval fixed_t<30, int32_t> operator"" _iq30(unsigned long long x) { return fixed_t<30, int32_t>(x); }
consteval fixed_t<31, int32_t> operator"" _iq31(unsigned long long x) { return fixed_t<31, int32_t>(x); }



consteval fixed_t<1, uint32_t> operator"" _uq1(long double x) { return fixed_t<1, uint32_t>(x); }
consteval fixed_t<2, uint32_t> operator"" _uq2(long double x) { return fixed_t<2, uint32_t>(x); }
consteval fixed_t<3, uint32_t> operator"" _uq3(long double x) { return fixed_t<3, uint32_t>(x); }
consteval fixed_t<4, uint32_t> operator"" _uq4(long double x) { return fixed_t<4, uint32_t>(x); }
consteval fixed_t<5, uint32_t> operator"" _uq5(long double x) { return fixed_t<5, uint32_t>(x); }
consteval fixed_t<6, uint32_t> operator"" _uq6(long double x) { return fixed_t<6, uint32_t>(x); }
consteval fixed_t<7, uint32_t> operator"" _uq7(long double x) { return fixed_t<7, uint32_t>(x); }
consteval fixed_t<8, uint32_t> operator"" _uq8(long double x) { return fixed_t<8, uint32_t>(x); }
consteval fixed_t<9, uint32_t> operator"" _uq9(long double x) { return fixed_t<9, uint32_t>(x); }
consteval fixed_t<10, uint32_t> operator"" _uq10(long double x) { return fixed_t<10, uint32_t>(x); }
consteval fixed_t<11, uint32_t> operator"" _uq11(long double x) { return fixed_t<11, uint32_t>(x); }
consteval fixed_t<12, uint32_t> operator"" _uq12(long double x) { return fixed_t<12, uint32_t>(x); }
consteval fixed_t<13, uint32_t> operator"" _uq13(long double x) { return fixed_t<13, uint32_t>(x); }
consteval fixed_t<14, uint32_t> operator"" _uq14(long double x) { return fixed_t<14, uint32_t>(x); }
consteval fixed_t<15, uint32_t> operator"" _uq15(long double x) { return fixed_t<15, uint32_t>(x); }
consteval fixed_t<16, uint32_t> operator"" _uq16(long double x) { return fixed_t<16, uint32_t>(x); }
consteval fixed_t<17, uint32_t> operator"" _uq17(long double x) { return fixed_t<17, uint32_t>(x); }
consteval fixed_t<18, uint32_t> operator"" _uq18(long double x) { return fixed_t<18, uint32_t>(x); }
consteval fixed_t<19, uint32_t> operator"" _uq19(long double x) { return fixed_t<19, uint32_t>(x); }
consteval fixed_t<20, uint32_t> operator"" _uq20(long double x) { return fixed_t<20, uint32_t>(x); }
consteval fixed_t<21, uint32_t> operator"" _uq21(long double x) { return fixed_t<21, uint32_t>(x); }
consteval fixed_t<22, uint32_t> operator"" _uq22(long double x) { return fixed_t<22, uint32_t>(x); }
consteval fixed_t<23, uint32_t> operator"" _uq23(long double x) { return fixed_t<23, uint32_t>(x); }
consteval fixed_t<24, uint32_t> operator"" _uq24(long double x) { return fixed_t<24, uint32_t>(x); }
consteval fixed_t<25, uint32_t> operator"" _uq25(long double x) { return fixed_t<25, uint32_t>(x); }
consteval fixed_t<26, uint32_t> operator"" _uq26(long double x) { return fixed_t<26, uint32_t>(x); }
consteval fixed_t<27, uint32_t> operator"" _uq27(long double x) { return fixed_t<27, uint32_t>(x); }
consteval fixed_t<28, uint32_t> operator"" _uq28(long double x) { return fixed_t<28, uint32_t>(x); }
consteval fixed_t<29, uint32_t> operator"" _uq29(long double x) { return fixed_t<29, uint32_t>(x); }
consteval fixed_t<30, uint32_t> operator"" _uq30(long double x) { return fixed_t<30, uint32_t>(x); }
consteval fixed_t<31, uint32_t> operator"" _uq31(long double x) { return fixed_t<31, uint32_t>(x); }
consteval fixed_t<32, uint32_t> operator"" _uq32(long double x) { return fixed_t<32, uint32_t>(x); }

consteval fixed_t<1, uint32_t> operator"" _uq1(unsigned long long x) { return fixed_t<1, uint32_t>(x); }
consteval fixed_t<2, uint32_t> operator"" _uq2(unsigned long long x) { return fixed_t<2, uint32_t>(x); }
consteval fixed_t<3, uint32_t> operator"" _uq3(unsigned long long x) { return fixed_t<3, uint32_t>(x); }
consteval fixed_t<4, uint32_t> operator"" _uq4(unsigned long long x) { return fixed_t<4, uint32_t>(x); }
consteval fixed_t<5, uint32_t> operator"" _uq5(unsigned long long x) { return fixed_t<5, uint32_t>(x); }
consteval fixed_t<6, uint32_t> operator"" _uq6(unsigned long long x) { return fixed_t<6, uint32_t>(x); }
consteval fixed_t<7, uint32_t> operator"" _uq7(unsigned long long x) { return fixed_t<7, uint32_t>(x); }
consteval fixed_t<8, uint32_t> operator"" _uq8(unsigned long long x) { return fixed_t<8, uint32_t>(x); }
consteval fixed_t<9, uint32_t> operator"" _uq9(unsigned long long x) { return fixed_t<9, uint32_t>(x); }
consteval fixed_t<10, uint32_t> operator"" _uq10(unsigned long long x) { return fixed_t<10, uint32_t>(x); }
consteval fixed_t<11, uint32_t> operator"" _uq11(unsigned long long x) { return fixed_t<11, uint32_t>(x); }
consteval fixed_t<12, uint32_t> operator"" _uq12(unsigned long long x) { return fixed_t<12, uint32_t>(x); }
consteval fixed_t<13, uint32_t> operator"" _uq13(unsigned long long x) { return fixed_t<13, uint32_t>(x); }
consteval fixed_t<14, uint32_t> operator"" _uq14(unsigned long long x) { return fixed_t<14, uint32_t>(x); }
consteval fixed_t<15, uint32_t> operator"" _uq15(unsigned long long x) { return fixed_t<15, uint32_t>(x); }
consteval fixed_t<16, uint32_t> operator"" _uq16(unsigned long long x) { return fixed_t<16, uint32_t>(x); }
consteval fixed_t<17, uint32_t> operator"" _uq17(unsigned long long x) { return fixed_t<17, uint32_t>(x); }
consteval fixed_t<18, uint32_t> operator"" _uq18(unsigned long long x) { return fixed_t<18, uint32_t>(x); }
consteval fixed_t<19, uint32_t> operator"" _uq19(unsigned long long x) { return fixed_t<19, uint32_t>(x); }
consteval fixed_t<20, uint32_t> operator"" _uq20(unsigned long long x) { return fixed_t<20, uint32_t>(x); }
consteval fixed_t<21, uint32_t> operator"" _uq21(unsigned long long x) { return fixed_t<21, uint32_t>(x); }
consteval fixed_t<22, uint32_t> operator"" _uq22(unsigned long long x) { return fixed_t<22, uint32_t>(x); }
consteval fixed_t<23, uint32_t> operator"" _uq23(unsigned long long x) { return fixed_t<23, uint32_t>(x); }
consteval fixed_t<24, uint32_t> operator"" _uq24(unsigned long long x) { return fixed_t<24, uint32_t>(x); }
consteval fixed_t<25, uint32_t> operator"" _uq25(unsigned long long x) { return fixed_t<25, uint32_t>(x); }
consteval fixed_t<26, uint32_t> operator"" _uq26(unsigned long long x) { return fixed_t<26, uint32_t>(x); }
consteval fixed_t<27, uint32_t> operator"" _uq27(unsigned long long x) { return fixed_t<27, uint32_t>(x); }
consteval fixed_t<28, uint32_t> operator"" _uq28(unsigned long long x) { return fixed_t<28, uint32_t>(x); }
consteval fixed_t<29, uint32_t> operator"" _uq29(unsigned long long x) { return fixed_t<29, uint32_t>(x); }
consteval fixed_t<30, uint32_t> operator"" _uq30(unsigned long long x) { return fixed_t<30, uint32_t>(x); }
consteval fixed_t<31, uint32_t> operator"" _uq31(unsigned long long x) { return fixed_t<31, uint32_t>(x); }
consteval fixed_t<32, uint32_t> operator"" _uq32(unsigned long long x) { return fixed_t<32, uint32_t>(x); }
}
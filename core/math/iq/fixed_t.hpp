#pragma once

#include <compare>
#include "core/platform.hpp"

#include "details/support.hpp"
#include "details/_IQNdiv.hpp"
#include "details/_IQNconv.hpp"

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
#define IQ_DEFAULT_Q (size_t(16))
#endif



namespace ymd{
template<size_t Q, integral D>
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


}


namespace tmp{
template<typename U>
struct extended_underlying_type;

template<>
struct extended_underlying_type<uint8_t>{
    using type = uint16_t;
};

template<>
struct extended_underlying_type<uint16_t>{
    using type = uint32_t;
};

template<>
struct extended_underlying_type<uint32_t>{
    using type = uint64_t;
};

template<>
struct extended_underlying_type<size_t>{
    using type = uint64_t;
};

template<>
struct extended_underlying_type<uint64_t>{
    using type = uint64_t;
};

template<>
struct extended_underlying_type<int8_t>{
    using type = int16_t;
};

template<>
struct extended_underlying_type<int16_t>{
    using type = int32_t;
};

template<>
struct extended_underlying_type<int32_t>{
    using type = int64_t;
};

template<>
struct extended_underlying_type<int>{
    using type = int64_t;
};

template<>
struct extended_underlying_type<int64_t>{
    using type = int64_t;
};

template<integral T1, integral T2>
struct mul_underlying_type{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;
    using unsigned_t1_type = std::make_unsigned_t<T1>;
    using unsigned_t2_type = std::make_unsigned_t<T2>;

    using bigger_uint_type = std::conditional_t<
        (sizeof(unsigned_t1_type) > sizeof(unsigned_t2_type)),
        unsigned_t1_type, unsigned_t2_type
    >;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<bigger_uint_type>,
        bigger_uint_type
    >;
};

template<integral T1, integral T2>
struct sum_underlying_type{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;
    using unsigned_t1_type = std::make_unsigned_t<T1>;
    using unsigned_t2_type = std::make_unsigned_t<T2>;

    using bigger_uint_type = std::conditional_t<
        (sizeof(unsigned_t1_type) > sizeof(unsigned_t2_type)),
        unsigned_t1_type, unsigned_t2_type
    >;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<bigger_uint_type>,
        bigger_uint_type
    >;
};


static_assert(std::is_same_v<typename mul_underlying_type<int32_t, int32_t>::type, int32_t>);
static_assert(std::is_same_v<typename mul_underlying_type<long int, long int>::type, long int>);

template<integral T1, integral T2>
struct extended_mul_underlying_type{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;
    using unsigned_t1_type = std::make_unsigned_t<T1>;
    using unsigned_t2_type = std::make_unsigned_t<T2>;

    using bigger_uint_type = std::conditional_t<
        (sizeof(unsigned_t1_type) > sizeof(unsigned_t2_type)),
        unsigned_t1_type, unsigned_t2_type
    >;

    using extended_bigger_uint_type = typename extended_underlying_type<bigger_uint_type>::type;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<extended_bigger_uint_type>,
        extended_bigger_uint_type
    >;
};

template<integral T1, integral T2>
using extended_mul_underlying_t = typename extended_mul_underlying_type<T1, T2>::type;

template<typename T>
using extended_underlying_t = typename extended_underlying_type<T>::type;

template<integral T1, integral T2>
using mul_underlying_t = typename mul_underlying_type<T1, T2>::type;

template<integral T1, integral T2>
using sum_underlying_t = typename sum_underlying_type<T1, T2>::type;


}


template<size_t Q, integral D>
struct [[nodiscard]] fixed_t{
private:
    static_assert(std::is_same_v<D, bool> == false);
    static constexpr size_t MAX_Q = std::is_unsigned_v<D> ? 
        size_t(sizeof(D) * 8) : 
        size_t(sizeof(D) * 8 - 1); // 为符号位预留一个bit
    static_assert(Q <= MAX_Q);

    D count_;

public:
    struct count_ctor{
        D count;
    };

    template<size_t P>
    [[nodiscard]] __fast_inline static constexpr D transform(const D value){
        if constexpr (P > Q){
            return D(static_cast<tmp::extended_underlying_t<D>>(value) << (P - Q));
        }else if constexpr (P < Q){
            return D(static_cast<tmp::extended_underlying_t<D>>(value) >> (Q - P));
        }else{
            return value;
        }
    }

    static constexpr size_t q_num = Q;

    __fast_inline constexpr fixed_t(){;}

    __fast_inline constexpr fixed_t(count_ctor ctor):
        count_(ctor.count){;}

    [[nodiscard]] static constexpr fixed_t<Q, D> from_bits(const D count){
        return fixed_t<Q, D>(count_ctor{count});
    };

    __fast_inline constexpr fixed_t(const fixed_t<Q, D> & other):
        count_(other.count_){};


    template<size_t P, typename U>
    requires (sizeof(D) == sizeof(U))
    __fast_inline constexpr operator fixed_t<P, U>() const {
        const auto bits = static_cast<U>(transform<P>(count_));
        return fixed_t<P, U>::from_bits(bits);
    }

    __fast_inline constexpr D to_bits() const {return count_;}
    
    template<size_t P>
    __fast_inline constexpr fixed_t & operator = (const fixed_t<P, D> & other){
        count_ = fixed_t<Q, D>::template transform<Q>(other.to_bits());
        return *this;
    };

    template<size_t P>
    __fast_inline constexpr fixed_t & operator = (fixed_t<P, D> && other){
        count_ = fixed_t<Q, D>::template transform<Q>(other.to_bits());
        return *this;
    };
    

    template<typename T>
    requires (std::is_integral_v<T> and (not std::is_same_v<T, bool>))
    __fast_inline constexpr fixed_t(const T iv):
        fixed_t(count_ctor{static_cast<D>(
            iv * static_cast<tmp::extended_underlying_t<T>>(uint64_t(1) << Q)
        )}){;}

    __fast_inline consteval explicit fixed_t(const long double dv):
        fixed_t(count_ctor{static_cast<D>(dv * static_cast<long double>(uint64_t(1) << Q))}){};

    __fast_inline static constexpr fixed_t from (const floating auto fv){
        return fixed_t{count_ctor{
            .count = static_cast<D>(iqmath::details::_IQFtoN<Q>(fv))}
        };
    }

    __fast_inline constexpr fixed_t operator+() const {
        return *this;
    }

    __fast_inline constexpr fixed_t operator-() const {
        return fixed_t::from_bits(-(to_bits()));
    }

    template<integral D2>
    __fast_inline constexpr fixed_t<Q, D2> cast_inner() const {
        return fixed_t<Q, D2>::from_bits(static_cast<D2>(to_bits()));
    }

    //#region addsub
    template<size_t P>
    __fast_inline constexpr fixed_t & operator +=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(this->to_bits() + fixed_t<Q, D>(other).to_bits());
    }

    __fast_inline constexpr fixed_t & operator +=(const integral auto other) {
        return *this += fixed_t<Q, D>(other);
    }


    template<size_t P>
    __fast_inline constexpr fixed_t & operator -=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(this->to_bits() - fixed_t<Q, D>(other).to_bits());
    }

    __fast_inline constexpr fixed_t & operator -=(const integral auto other) {
        return *this -= fixed_t<Q, D>(other);
    }
    //#endregion addsub

    //#region multiply

    template<size_t P>
    __fast_inline constexpr fixed_t& operator *=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(
            (static_cast<tmp::extended_underlying_t<D>>(this->to_bits()) * 
            static_cast<tmp::extended_underlying_t<D>>((other).to_bits())) >> (P)
        );
    }

    __fast_inline constexpr fixed_t& operator *=(const integral auto other) {
        return *this = fixed_t<Q, D>::from_bits(this->to_bits() * other);
    }
    
    //#endregion

    //#region division
    __fast_inline constexpr fixed_t & operator/=(const integral auto other) {
        return *this = fixed_t::from_bits((to_bits() / D(other)));
    }

    template<size_t P>
    __fast_inline constexpr fixed_t & operator/=(const fixed_t<P, D> other) {
        if (std::is_constant_evaluated()) {
            return *this = fixed_t<Q, D>::from(float(*this) / float(other));
        }else{
            return *this = fixed_t::from_bits(iqmath::details::__IQNdiv_impl<Q, false>(
                to_bits(), other.to_bits()
            ));
        }
    }

    //#endregion

    //#region comparisons



    //#endregion

    //#region shifts
    [[nodiscard]] __fast_inline constexpr fixed_t operator<<(size_t shift) const {
        return fixed_t::from_bits((this->to_bits() << shift));
    }

    [[nodiscard]] __fast_inline constexpr fixed_t operator>>(size_t shift) const {
        return fixed_t::from_bits((this->to_bits() >> shift));
    }
    //#endregion

    [[nodiscard]] __fast_inline constexpr explicit operator bool() const {
        return bool(this->to_bits());
    }

    template<typename T>
    requires std::is_integral_v<T>
    [[nodiscard]] __fast_inline constexpr explicit operator T() const {
        return this->to_bits() >> Q;
    }
    

    template<typename T>
    requires std::is_floating_point_v<T>
    __inline constexpr explicit operator T() const{
        if(std::is_constant_evaluated()){
            return float(this->to_bits()) / D(1u << Q);
        }else{
            return iqmath::details::_IQNtoF<Q>(this->to_bits());
        }
    }
};

template<size_t Q, size_t P, integral D1, integral D2, typename D = tmp::sum_underlying_t<D1, D2>>
__fast_inline constexpr fixed_t<Q, D> operator +(const fixed_t<Q, D1> lhs, const fixed_t<P, D2> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(rhs);
    ret += fixed_t<Q, D>(lhs);
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

template<size_t Q, size_t P, integral D1, integral D2, typename D = tmp::sum_underlying_t<D1, D2>>
__fast_inline constexpr fixed_t<Q, D> operator -(const fixed_t<Q, D1> lhs, const fixed_t<P, D2> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret -= fixed_t<Q, D>(rhs);
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


template<size_t Q1, size_t Q2, integral D1, integral D2, typename D = tmp::extended_mul_underlying_t<D1, D2>>
__fast_inline constexpr fixed_t<Q1, D1> operator *(const fixed_t<Q1, D1> lhs, const fixed_t<Q2, D2> rhs) {
    return fixed_t<Q1, D1>::from_bits(static_cast<D>(lhs.to_bits()) * static_cast<D>(rhs.to_bits()) >> Q2);
}

template<size_t Q, integral D1, integral D2, typename D = tmp::extended_mul_underlying_t<D1, D2>>
__fast_inline constexpr fixed_t<Q, D1> operator *(const D2 lhs, const fixed_t<Q, D1> rhs) {
    return fixed_t<Q, D1>::from_bits(static_cast<D>(lhs) * static_cast<D>(rhs.to_bits()));
}

template<size_t Q, integral D1, integral D2, typename D = tmp::extended_mul_underlying_t<D1, D2>>
__fast_inline constexpr fixed_t<Q, D1> operator *(const fixed_t<Q, D1> lhs, const D2 rhs) {
    return fixed_t<Q, D1>::from_bits(static_cast<D>(lhs.to_bits()) * static_cast<D>(rhs));
}

template<size_t Q, integral D>
__fast_inline constexpr fixed_t<Q, D> operator *(const bool lhs, const fixed_t<Q, D> rhs) {
    return fixed_t<Q, D>::from_bits(lhs * (rhs.to_bits()));
}

template<size_t Q, integral D>
__fast_inline constexpr fixed_t<Q, D> operator *(const fixed_t<Q, D> lhs, const bool rhs) {
    return fixed_t<Q, D>::from_bits((lhs.to_bits()) * rhs);
}

template<size_t Q, size_t P, integral D1, integral D2>
__fast_inline constexpr fixed_t<Q, D1> operator /(const fixed_t<Q, D1> lhs, const fixed_t<P, D2> rhs) {
    static_assert(sizeof(D1) == sizeof(D2));
    fixed_t<Q, D1> ret = lhs;
    ret /= rhs.template cast_inner<D1>();
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


#if 0
template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, U> abs(const fixed_t<Q, D> x){
    const auto bits = x.to_bits();
    return fixed_t<Q, U>::from_bits(static_cast<U>(bits > 0 ? bits : -bits));
}
#else
template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> abs(const fixed_t<Q, D> x){
    const auto bits = x.to_bits();
    return fixed_t<Q, D>::from_bits(static_cast<D>(bits > 0 ? bits : -bits));
}
#endif


template<size_t Q1, size_t Q2, integral D1, integral D2>
[[nodiscard]] __fast_inline constexpr 
std::strong_ordering operator <=> (const fixed_t<Q1, D1> & self, const fixed_t<Q2, D2> & other) {
    if constexpr(std::is_same_v<D1, D2>){
        if constexpr(Q2 == Q1){
            return self.to_bits() <=> (other.to_bits());
        }else{
            using u_t = tmp::extended_underlying_t<D1>;
            return (static_cast<u_t>(self.to_bits()) << Q2) <=> (static_cast<u_t>(other.to_bits()) << Q1);
        }
    }else{
        using u_t = tmp::extended_underlying_t<D1>;
        return (static_cast<u_t>(self.to_bits()) << Q2) <=> (static_cast<u_t>(other.to_bits()) << Q1);
    }
}



template<size_t Q, integral D, integral T>
[[nodiscard]] __fast_inline constexpr 
std::strong_ordering operator <=> (const fixed_t<Q, D> & self, const T & other) {
    return (((self.to_bits())) <=> (D(other) << Q));
}
template<size_t Q, integral D, integral T>
[[nodiscard]] __fast_inline constexpr 
std::strong_ordering operator <=> (const T & other, const fixed_t<Q,D> & self){
    return (((self.to_bits())) <=> (D(other) << Q));
}

// 统一的等于运算符模板，直接复用 <=>
template<size_t Q, integral D, integral T>
[[nodiscard]] __fast_inline constexpr 
bool operator == (const fixed_t<Q, D> & self, const T & other) {
    return (self <=> other) == 0;
}

template<size_t Q, integral D, integral T>
[[nodiscard]] __fast_inline constexpr 
bool operator == (const T & other, const fixed_t<Q, D> & self) {
    return (self <=> other) == 0;  // 注意这里复用 self <=> other
}


template<size_t Q1, size_t Q2, integral D1, integral D2>
[[nodiscard]] __fast_inline constexpr 
bool operator == (const fixed_t<Q1, D1> & self, const fixed_t<Q2, D2> & other) {
    return (self <=> other) == 0;  // 注意这里复用 self <=> other
}


template<size_t Q1, size_t Q2, integral D1, integral D2, typename D = tmp::extended_mul_underlying_t<D1, D2>>
static constexpr auto extended_mul(const fixed_t<Q1, D1> a, const fixed_t<Q2, D2> b) -> fixed_t<Q1 + Q2, D>{
    return fixed_t<Q1 + Q2, D>::from_bits(static_cast<D>(a.to_bits()) * static_cast<D>(b.to_bits()));
}

template<size_t Q1, size_t Q2>
static constexpr fixed_t<Q1, int32_t> sat(const fixed_t<Q1, int32_t> x, const fixed_t<Q2, int32_t> k){
    const auto kx = extended_mul(x, k);
    constexpr auto mask = (std::numeric_limits<uint64_t>::max() << (Q1 + Q2));
    if(kx.to_bits() & mask){
        if(kx < 0) return -1;
        else return 1;
    }else{
        return fixed_t<Q1, int32_t>::from_bits(kx.to_bits() >> Q2);
    }
}


template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr bool signbit(const fixed_t<Q, D> x){
    constexpr D SIGN_MASK = static_cast<D>(static_cast<D>(1) << size_t(sizeof(D) * 8 - 1));
    return static_cast<bool>(x.to_bits() & SIGN_MASK);
}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> sign(const fixed_t<Q, D> x){
    static_assert(fixed_t<Q, D>(-1) != fixed_t<Q, D>(1));
    if(likely(x)) return fixed_t<Q, D>(x > 0 ? 1 : -1);
    else return fixed_t<Q, D>(0);
}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> fmod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits(a.to_bits() % b.to_bits()));
}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, U> fposmod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    constexpr size_t SHIFT = size_t(sizeof(D) * 8 - 1);
    const D rem = std::bit_cast<D>(a.to_bits() % b.to_bits());
    const D is_negative = static_cast<D>(rem >> SHIFT);  // 符号位扩展（0 或 -1）
    return fixed_t<Q, U>::from_bits(static_cast<U>(rem + static_cast<U>(b.to_bits() & is_negative)));
}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> lerp(const fixed_t<Q, D> x, const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return a * (1 - x) + b * x;
}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> mean(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits((a.to_bits() + b.to_bits()) >> 1));}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, U> frac(const fixed_t<Q, D> x){
    static constexpr U MASK = static_cast<U>((uint64_t(1u) << Q) - 1);
    return fixed_t<Q, U>::from_bits(static_cast<U>(static_cast<U>(x.to_bits()) & MASK));
}


template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr D floor_int(const fixed_t<Q, D> x){
    return static_cast<D>(x.to_bits() >> Q);}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr D ceil_int(const fixed_t<Q, D> x){
    static constexpr D MASK = (1 << Q) - 1;
    return static_cast<D>((x.to_bits() >> Q) + bool(x.to_bits() & MASK));
}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr D round_int(const fixed_t<Q, D> x){
    static constexpr D MASK = (1 << (Q - 1));
    return static_cast<D>((x.to_bits() + MASK) >> Q);
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr T floor_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(floor_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr T ceil_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(ceil_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr T round_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(round_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> floor(const fixed_t<Q, D> x){
    return floor_int(x);
}

template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> ceil(const fixed_t<Q, D> x){
    return ceil_int(x);
}



template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> round(const fixed_t<Q, D> x){
    return round_int(x);
}



template<size_t Q, typename D>
[[nodiscard]] __fast_inline
constexpr fixed_t<Q, D> square(const fixed_t<Q, D> x) {
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

[[nodiscard]] consteval iq1 operator"" _iq1(long double x) { return iq1(x); }
[[nodiscard]] consteval iq2 operator"" _iq2(long double x) { return iq2(x); }
[[nodiscard]] consteval iq3 operator"" _iq3(long double x) { return iq3(x); }
[[nodiscard]] consteval iq4 operator"" _iq4(long double x) { return iq4(x); }
[[nodiscard]] consteval iq5 operator"" _iq5(long double x) { return iq5(x); }
[[nodiscard]] consteval iq6 operator"" _iq6(long double x) { return iq6(x); }
[[nodiscard]] consteval iq7 operator"" _iq7(long double x) { return iq7(x); }
[[nodiscard]] consteval iq8 operator"" _iq8(long double x) { return iq8(x); }
[[nodiscard]] consteval iq9 operator"" _iq9(long double x) { return iq9(x); }
[[nodiscard]] consteval iq10 operator"" _iq10(long double x) { return iq10(x); }
[[nodiscard]] consteval iq11 operator"" _iq11(long double x) { return iq11(x); }
[[nodiscard]] consteval iq12 operator"" _iq12(long double x) { return iq12(x); }
[[nodiscard]] consteval iq13 operator"" _iq13(long double x) { return iq13(x); }
[[nodiscard]] consteval iq14 operator"" _iq14(long double x) { return iq14(x); }
[[nodiscard]] consteval iq15 operator"" _iq15(long double x) { return iq15(x); }
[[nodiscard]] consteval iq16 operator"" _iq16(long double x) { return iq16(x); }
[[nodiscard]] consteval iq17 operator"" _iq17(long double x) { return iq17(x); }
[[nodiscard]] consteval iq18 operator"" _iq18(long double x) { return iq18(x); }
[[nodiscard]] consteval iq19 operator"" _iq19(long double x) { return iq19(x); }
[[nodiscard]] consteval iq20 operator"" _iq20(long double x) { return iq20(x); }
[[nodiscard]] consteval iq21 operator"" _iq21(long double x) { return iq21(x); }
[[nodiscard]] consteval iq22 operator"" _iq22(long double x) { return iq22(x); }
[[nodiscard]] consteval iq23 operator"" _iq23(long double x) { return iq23(x); }
[[nodiscard]] consteval iq24 operator"" _iq24(long double x) { return iq24(x); }
[[nodiscard]] consteval iq25 operator"" _iq25(long double x) { return iq25(x); }
[[nodiscard]] consteval iq26 operator"" _iq26(long double x) { return iq26(x); }
[[nodiscard]] consteval iq27 operator"" _iq27(long double x) { return iq27(x); }
[[nodiscard]] consteval iq28 operator"" _iq28(long double x) { return iq28(x); }
[[nodiscard]] consteval iq29 operator"" _iq29(long double x) { return iq29(x); }
[[nodiscard]] consteval iq30 operator"" _iq30(long double x) { return iq30(x); }
[[nodiscard]] consteval iq31 operator"" _iq31(long double x) { return iq31(x); }

[[nodiscard]] consteval iq1 operator"" _iq1(unsigned long long x) { return iq1(x); }
[[nodiscard]] consteval iq2 operator"" _iq2(unsigned long long x) { return iq2(x); }
[[nodiscard]] consteval iq3 operator"" _iq3(unsigned long long x) { return iq3(x); }
[[nodiscard]] consteval iq4 operator"" _iq4(unsigned long long x) { return iq4(x); }
[[nodiscard]] consteval iq5 operator"" _iq5(unsigned long long x) { return iq5(x); }
[[nodiscard]] consteval iq6 operator"" _iq6(unsigned long long x) { return iq6(x); }
[[nodiscard]] consteval iq7 operator"" _iq7(unsigned long long x) { return iq7(x); }
[[nodiscard]] consteval iq8 operator"" _iq8(unsigned long long x) { return iq8(x); }
[[nodiscard]] consteval iq9 operator"" _iq9(unsigned long long x) { return iq9(x); }
[[nodiscard]] consteval iq10 operator"" _iq10(unsigned long long x) { return iq10(x); }
[[nodiscard]] consteval iq11 operator"" _iq11(unsigned long long x) { return iq11(x); }
[[nodiscard]] consteval iq12 operator"" _iq12(unsigned long long x) { return iq12(x); }
[[nodiscard]] consteval iq13 operator"" _iq13(unsigned long long x) { return iq13(x); }
[[nodiscard]] consteval iq14 operator"" _iq14(unsigned long long x) { return iq14(x); }
[[nodiscard]] consteval iq15 operator"" _iq15(unsigned long long x) { return iq15(x); }
[[nodiscard]] consteval iq16 operator"" _iq16(unsigned long long x) { return iq16(x); }
[[nodiscard]] consteval iq17 operator"" _iq17(unsigned long long x) { return iq17(x); }
[[nodiscard]] consteval iq18 operator"" _iq18(unsigned long long x) { return iq18(x); }
[[nodiscard]] consteval iq19 operator"" _iq19(unsigned long long x) { return iq19(x); }
[[nodiscard]] consteval iq20 operator"" _iq20(unsigned long long x) { return iq20(x); }
[[nodiscard]] consteval iq21 operator"" _iq21(unsigned long long x) { return iq21(x); }
[[nodiscard]] consteval iq22 operator"" _iq22(unsigned long long x) { return iq22(x); }
[[nodiscard]] consteval iq23 operator"" _iq23(unsigned long long x) { return iq23(x); }
[[nodiscard]] consteval iq24 operator"" _iq24(unsigned long long x) { return iq24(x); }
[[nodiscard]] consteval iq25 operator"" _iq25(unsigned long long x) { return iq25(x); }
[[nodiscard]] consteval iq26 operator"" _iq26(unsigned long long x) { return iq26(x); }
[[nodiscard]] consteval iq27 operator"" _iq27(unsigned long long x) { return iq27(x); }
[[nodiscard]] consteval iq28 operator"" _iq28(unsigned long long x) { return iq28(x); }
[[nodiscard]] consteval iq29 operator"" _iq29(unsigned long long x) { return iq29(x); }
[[nodiscard]] consteval iq30 operator"" _iq30(unsigned long long x) { return iq30(x); }
[[nodiscard]] consteval iq31 operator"" _iq31(unsigned long long x) { return iq31(x); }



[[nodiscard]] consteval uq1 operator"" _uq1(long double x) { return uq1(x); }
[[nodiscard]] consteval uq2 operator"" _uq2(long double x) { return uq2(x); }
[[nodiscard]] consteval uq3 operator"" _uq3(long double x) { return uq3(x); }
[[nodiscard]] consteval uq4 operator"" _uq4(long double x) { return uq4(x); }
[[nodiscard]] consteval uq5 operator"" _uq5(long double x) { return uq5(x); }
[[nodiscard]] consteval uq6 operator"" _uq6(long double x) { return uq6(x); }
[[nodiscard]] consteval uq7 operator"" _uq7(long double x) { return uq7(x); }
[[nodiscard]] consteval uq8 operator"" _uq8(long double x) { return uq8(x); }
[[nodiscard]] consteval uq9 operator"" _uq9(long double x) { return uq9(x); }
[[nodiscard]] consteval uq10 operator"" _uq10(long double x) { return uq10(x); }
[[nodiscard]] consteval uq11 operator"" _uq11(long double x) { return uq11(x); }
[[nodiscard]] consteval uq12 operator"" _uq12(long double x) { return uq12(x); }
[[nodiscard]] consteval uq13 operator"" _uq13(long double x) { return uq13(x); }
[[nodiscard]] consteval uq14 operator"" _uq14(long double x) { return uq14(x); }
[[nodiscard]] consteval uq15 operator"" _uq15(long double x) { return uq15(x); }
[[nodiscard]] consteval uq16 operator"" _uq16(long double x) { return uq16(x); }
[[nodiscard]] consteval uq17 operator"" _uq17(long double x) { return uq17(x); }
[[nodiscard]] consteval uq18 operator"" _uq18(long double x) { return uq18(x); }
[[nodiscard]] consteval uq19 operator"" _uq19(long double x) { return uq19(x); }
[[nodiscard]] consteval uq20 operator"" _uq20(long double x) { return uq20(x); }
[[nodiscard]] consteval uq21 operator"" _uq21(long double x) { return uq21(x); }
[[nodiscard]] consteval uq22 operator"" _uq22(long double x) { return uq22(x); }
[[nodiscard]] consteval uq23 operator"" _uq23(long double x) { return uq23(x); }
[[nodiscard]] consteval uq24 operator"" _uq24(long double x) { return uq24(x); }
[[nodiscard]] consteval uq25 operator"" _uq25(long double x) { return uq25(x); }
[[nodiscard]] consteval uq26 operator"" _uq26(long double x) { return uq26(x); }
[[nodiscard]] consteval uq27 operator"" _uq27(long double x) { return uq27(x); }
[[nodiscard]] consteval uq28 operator"" _uq28(long double x) { return uq28(x); }
[[nodiscard]] consteval uq29 operator"" _uq29(long double x) { return uq29(x); }
[[nodiscard]] consteval uq30 operator"" _uq30(long double x) { return uq30(x); }
[[nodiscard]] consteval uq31 operator"" _uq31(long double x) { return uq31(x); }
[[nodiscard]] consteval uq32 operator"" _uq32(long double x) { return uq32(x); }

[[nodiscard]] consteval uq1 operator"" _uq1(unsigned long long x) { return uq1(x); }
[[nodiscard]] consteval uq2 operator"" _uq2(unsigned long long x) { return uq2(x); }
[[nodiscard]] consteval uq3 operator"" _uq3(unsigned long long x) { return uq3(x); }
[[nodiscard]] consteval uq4 operator"" _uq4(unsigned long long x) { return uq4(x); }
[[nodiscard]] consteval uq5 operator"" _uq5(unsigned long long x) { return uq5(x); }
[[nodiscard]] consteval uq6 operator"" _uq6(unsigned long long x) { return uq6(x); }
[[nodiscard]] consteval uq7 operator"" _uq7(unsigned long long x) { return uq7(x); }
[[nodiscard]] consteval uq8 operator"" _uq8(unsigned long long x) { return uq8(x); }
[[nodiscard]] consteval uq9 operator"" _uq9(unsigned long long x) { return uq9(x); }
[[nodiscard]] consteval uq10 operator"" _uq10(unsigned long long x) { return uq10(x); }
[[nodiscard]] consteval uq11 operator"" _uq11(unsigned long long x) { return uq11(x); }
[[nodiscard]] consteval uq12 operator"" _uq12(unsigned long long x) { return uq12(x); }
[[nodiscard]] consteval uq13 operator"" _uq13(unsigned long long x) { return uq13(x); }
[[nodiscard]] consteval uq14 operator"" _uq14(unsigned long long x) { return uq14(x); }
[[nodiscard]] consteval uq15 operator"" _uq15(unsigned long long x) { return uq15(x); }
[[nodiscard]] consteval uq16 operator"" _uq16(unsigned long long x) { return uq16(x); }
[[nodiscard]] consteval uq17 operator"" _uq17(unsigned long long x) { return uq17(x); }
[[nodiscard]] consteval uq18 operator"" _uq18(unsigned long long x) { return uq18(x); }
[[nodiscard]] consteval uq19 operator"" _uq19(unsigned long long x) { return uq19(x); }
[[nodiscard]] consteval uq20 operator"" _uq20(unsigned long long x) { return uq20(x); }
[[nodiscard]] consteval uq21 operator"" _uq21(unsigned long long x) { return uq21(x); }
[[nodiscard]] consteval uq22 operator"" _uq22(unsigned long long x) { return uq22(x); }
[[nodiscard]] consteval uq23 operator"" _uq23(unsigned long long x) { return uq23(x); }
[[nodiscard]] consteval uq24 operator"" _uq24(unsigned long long x) { return uq24(x); }
[[nodiscard]] consteval uq25 operator"" _uq25(unsigned long long x) { return uq25(x); }
[[nodiscard]] consteval uq26 operator"" _uq26(unsigned long long x) { return uq26(x); }
[[nodiscard]] consteval uq27 operator"" _uq27(unsigned long long x) { return uq27(x); }
[[nodiscard]] consteval uq28 operator"" _uq28(unsigned long long x) { return uq28(x); }
[[nodiscard]] consteval uq29 operator"" _uq29(unsigned long long x) { return uq29(x); }
[[nodiscard]] consteval uq30 operator"" _uq30(unsigned long long x) { return uq30(x); }
[[nodiscard]] consteval uq31 operator"" _uq31(unsigned long long x) { return uq31(x); }
[[nodiscard]] consteval uq32 operator"" _uq32(unsigned long long x) { return uq32(x); }
}
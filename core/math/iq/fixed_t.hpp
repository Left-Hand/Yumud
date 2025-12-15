#pragma once

#include <compare>
#include "core/tmp/integral.hpp"

#include "details/support.hpp"
#include "details/_IQNdiv.hpp"
#include "details/_IQNconv.hpp"

#include <numeric>
#include <cstddef>


#ifndef LOG_E
#define LOG_E (0.434294481903)
#endif

#ifndef IQ_DEFAULT_Q
#define IQ_DEFAULT_Q (size_t(16))
#endif


namespace ymd{
template<size_t Q, std::integral D>
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




template<size_t Q, std::integral D>
struct [[nodiscard]] fixed_t{
private:
    static_assert(std
        ::is_same_v<D, bool> == false);
    static constexpr size_t MAX_Q = std::is_unsigned_v<D> ? 
        size_t(sizeof(D) * 8) : 
        size_t(sizeof(D) * 8 - 1); // 为符号位预留一个bit
    static_assert(Q <= MAX_Q);

    using UD = std::make_unsigned_t<D>;
public:
    D bits;

    struct [[nodiscard]] bits_ctor{
        D count;
    };

    template<size_t P>
    [[nodiscard]] __attribute__((always_inline)) 
    static constexpr D transform(const D value){
        if constexpr (P > Q){
            return D(static_cast<tmp::extended_underlying_t<D>>(value) << (P - Q));
        }else if constexpr (P < Q){
            return D(static_cast<tmp::extended_underlying_t<D>>(value) >> (Q - P));
        }else{
            return value;
        }
    }

    static constexpr size_t q_num = Q;

    __attribute__((always_inline)) constexpr fixed_t(){;}

    __attribute__((always_inline)) constexpr explicit fixed_t(bits_ctor ctor):
        bits(ctor.count){;}

    [[nodiscard]] static constexpr fixed_t<Q, D> from_bits(const D bits){
        return fixed_t<Q, D>(bits_ctor{bits});
    };

    template<typename I>
    requires (
        //必须为非bool的整数类型
        std::is_integral_v<I> and (not std::is_same_v<I, bool>) 
        //如果D是无符号的 I也必须为无符号数
        and ((std::is_unsigned_v<D> ? std::is_unsigned_v<I> : true))
    )
    __attribute__((always_inline)) 
    [[nodiscard]] static constexpr fixed_t<Q, D> from_rcp(const I int_val){
        using ED = tmp::extended_underlying_t<D>;
        const auto bits = static_cast<ED>(static_cast<ED>(1u) << Q) / int_val;
        return fixed_t<Q, D>::from_bits(bits);
    };

    __attribute__((always_inline)) constexpr fixed_t(const fixed_t<Q, D> & other):
        bits(other.bits){};


    template<size_t P, typename U>
    requires (sizeof(D) == sizeof(U))
    __attribute__((always_inline)) constexpr operator fixed_t<P, U>() const {
        const auto new_bits = static_cast<U>(transform<P>(bits));
        return fixed_t<P, U>::from_bits(new_bits);
    }

    __attribute__((always_inline)) constexpr D to_bits() const {return bits;}
    
    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed_t & operator = (const fixed_t<P, D> & other){
        bits = fixed_t<Q, D>::template transform<Q>(other.to_bits());
        return *this;
    };

    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed_t & operator = (fixed_t<P, D> && other){
        bits = fixed_t<Q, D>::template transform<Q>(other.to_bits());
        return *this;
    };
    

    template<typename T>
    requires (std::is_integral_v<T> and (not std::is_same_v<T, bool>))
    __attribute__((always_inline)) constexpr 
    fixed_t(const T iv):
        fixed_t(bits_ctor{std::bit_cast<D>(static_cast<D>(
            iv * static_cast<tmp::extended_underlying_t<T>>(uint64_t(1) << Q)
        ))}){;}

    __attribute__((always_inline)) consteval explicit fixed_t(const long double dv):
        fixed_t(bits_ctor{std::bit_cast<D>(static_cast<D>(
            dv * static_cast<long double>(uint64_t(1) << Q)
        ))}){};

    __attribute__((always_inline)) static constexpr 
    fixed_t from (const std::floating_point auto fv){
        return fixed_t{bits_ctor{
            std::bit_cast<D>(static_cast<D>(iqmath::details::_IQFtoN<Q>(fv)))
        }};
    }

    __attribute__((always_inline)) constexpr 
    fixed_t operator+() const {
        return *this;
    }

    __attribute__((always_inline)) constexpr 
    fixed_t operator-() const 
    requires(std::is_signed_v<D>)
    {
        return fixed_t::from_bits(-(to_bits()));
    }

    template<std::integral D2>
    __attribute__((always_inline)) constexpr 
    fixed_t<Q, D2> cast_inner() const {
        return fixed_t<Q, D2>::from_bits(static_cast<D2>(to_bits()));
    }

    //#region addsub
    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed_t & operator +=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(this->to_bits() + fixed_t<Q, D>(other).to_bits());
    }

    __attribute__((always_inline)) constexpr 
    fixed_t & operator +=(const std::integral auto other) {
        return *this += fixed_t<Q, D>(other);
    }


    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed_t & operator -=(const fixed_t<P, D> other) {
        return *this = fixed_t<Q, D>::from_bits(this->to_bits() - fixed_t<Q, D>(other).to_bits());
    }

    __attribute__((always_inline)) constexpr 
    fixed_t & operator -=(const std::integral auto other) {
        return *this -= fixed_t<Q, D>(other);
    }
    //#endregion addsub

    //#region shifts
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    fixed_t operator<<(size_t shift) const {
        return fixed_t::from_bits((this->to_bits() << shift));
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    fixed_t operator>>(size_t shift) const {
        return fixed_t::from_bits((this->to_bits() >> shift));
    }
    //#endregion

    [[nodiscard]] __attribute__((always_inline)) constexpr explicit 
    operator bool() const {
        return bool(this->to_bits());
    }

    template<typename T>
    requires std::is_integral_v<T>
    [[nodiscard]] __attribute__((always_inline)) constexpr explicit 
    operator T() const {
        return this->to_bits() >> Q;
    }
    

    template<typename T>
    requires std::is_floating_point_v<T> __inline constexpr explicit 
    operator T() const{
        if(std::is_constant_evaluated()){
            return float(this->to_bits()) / D(1u << Q);
        }else{
            return iqmath::details::_IQNtoF<Q>(this->to_bits());
        }
    }
};

template<size_t Q, size_t P, 
    std::integral D1, std::integral D2, 
    typename D = tmp::sum_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator +(const fixed_t<Q, D1> lhs, const fixed_t<P, D2> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(rhs);
    ret += fixed_t<Q, D>(lhs);
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator +(const std::integral auto lhs, const fixed_t<Q, D> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret += rhs;
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator +(const fixed_t<Q, D> lhs, const std::integral auto rhs) {
    fixed_t<Q, D> ret = lhs;
    ret += fixed_t<Q, D>(rhs);
    return ret;
}

template<size_t Q, size_t P, 
    std::integral D1, std::integral D2, 
    typename D = tmp::sum_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator -(const fixed_t<Q, D1> lhs, const fixed_t<P, D2> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret -= fixed_t<Q, D>(rhs);
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator -(const std::integral auto lhs, const fixed_t<Q, D> rhs) {
    fixed_t<Q, D> ret = fixed_t<Q, D>(lhs);
    ret -= rhs;
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator -(const fixed_t<Q, D> lhs, const std::integral auto rhs) {
    fixed_t<Q, D> ret = lhs;
    ret -= fixed_t<Q, D>(rhs);
    return ret;
}


template<size_t Q1, size_t Q2, 
    std::integral D1, std::integral D2, 
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed_t<Q1, D> operator *(const fixed_t<Q1, D1> lhs, const fixed_t<Q2, D2> rhs) {
    using ED = tmp::extended_mul_underlying_t<D1, D2>;
    return fixed_t<Q1, D>::from_bits(
        static_cast<D>(static_cast<ED>(lhs.to_bits()) * static_cast<ED>(rhs.to_bits()) >> Q2)
    );
}

template<size_t Q, 
    std::integral D1, std::integral D2, 
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator *(const D2 lhs, const fixed_t<Q, D1> rhs) {
    return fixed_t<Q, D>::from_bits(
        static_cast<D>(lhs) * static_cast<D>(rhs.to_bits())
    );
}

template<size_t Q, 
    std::integral D1, std::integral D2, 
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator *(const fixed_t<Q, D1> lhs, const D2 rhs) {
    return fixed_t<Q, D>::from_bits(
        static_cast<D>(lhs.to_bits()) * static_cast<D>(rhs)
    );
}

template<size_t Q, std::integral D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator *(const bool lhs, const fixed_t<Q, D> rhs) {
    return lhs ? rhs : fixed_t<Q, D>::from_bits(0);
}

template<size_t Q, std::integral D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator *(const fixed_t<Q, D> lhs, const bool rhs) {
    return rhs ? lhs : fixed_t<Q, D>::from_bits(0);
}

template<
    size_t Q, size_t P, 
    std::integral D1, std::integral D2,
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator /(const fixed_t<Q, D1> lhs, const fixed_t<P, D2> rhs) {
    static_assert(sizeof(D1) == sizeof(D2));
    if (std::is_constant_evaluated()) {
        return fixed_t<Q, D>::from(float(lhs) / float(rhs));
    }else{
        return fixed_t<Q, D>::from_bits(iqmath::details::__IQNdiv_impl<Q, true>(
            lhs.to_bits(), rhs.to_bits()
        ));
    }
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator /(const std::integral auto lhs, const fixed_t<Q, D> rhs) {
	return fixed_t<Q, D>(lhs) / rhs;
}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> operator /(const fixed_t<Q, D> lhs, const std::integral auto rhs) {
	return fixed_t<Q, D>::from_bits(lhs.to_bits() / rhs);
}



template<size_t Q1, size_t Q2, std::integral D1, std::integral D2>
[[nodiscard]] __attribute__((always_inline)) constexpr 
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



template<size_t Q, std::integral D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
std::strong_ordering operator <=> (const fixed_t<Q, D> & self, const T & other) {
    return (std::bit_cast<D>(self.to_bits()) <=> (D(other) << Q));
}
template<size_t Q, std::integral D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
std::strong_ordering operator <=> (const T & other, const fixed_t<Q,D> & self){
    return (std::bit_cast<D>(self.to_bits()) <=> (D(other) << Q));
}

// 统一的等于运算符模板，直接复用 <=>
template<size_t Q, std::integral D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool operator == (const fixed_t<Q, D> & self, const T & other) {
    return (self <=> other) == 0;
}

template<size_t Q, std::integral D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool operator == (const T & other, const fixed_t<Q, D> & self) {
    return (self <=> other) == 0;  // 注意这里复用 self <=> other
}


template<size_t Q1, size_t Q2, std::integral D1, std::integral D2>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool operator == (const fixed_t<Q1, D1> & self, const fixed_t<Q2, D2> & other) {
    return (self <=> other) == 0;  // 注意这里复用 self <=> other
}


template<size_t Q1, size_t Q2, 
    std::integral D1, std::integral D2, 
    typename D = tmp::extended_mul_underlying_t<D1, D2>
    >
static constexpr auto extended_mul(const fixed_t<Q1, D1> a, const fixed_t<Q2, D2> b) 
-> fixed_t<Q1 + Q2, D>{
    return fixed_t<Q1 + Q2, D>::from_bits(static_cast<D>(a.to_bits()) * static_cast<D>(b.to_bits()));
}

namespace math{

template<size_t Q1, size_t Q2>
static constexpr fixed_t<Q1, int32_t> sat(
    const fixed_t<Q1, int32_t> x, const fixed_t<Q2, int32_t> k
){
    const auto kx = extended_mul(x, k);
    constexpr auto mask = (std::numeric_limits<uint64_t>::max() << (Q1 + Q2));
    if(kx.to_bits() & mask){
        if(kx < 0) return -1;
        else return 1;
    }else{
        return fixed_t<Q1, int32_t>::from_bits(kx.to_bits() >> Q2);
    }
}

#if 0
template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, U> abs(const fixed_t<Q, D> x){
    const auto bits = x.to_bits();
    return fixed_t<Q, U>::from_bits(static_cast<U>(bits > 0 ? bits : -bits));
}
#else
template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> abs(const fixed_t<Q, D> x){
    const auto bits = x.to_bits();
    return fixed_t<Q, D>::from_bits(static_cast<D>(bits > 0 ? bits : -bits));
}
#endif

template<size_t Q, typename D>
static constexpr fixed_t<Q, D> closer_to_zero(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    if constexpr(std::is_signed_v<D>){
        if(math::abs(a) < math::abs(b)){
            return a;
        }else{
            return b;
        }
    }else{
        if(a < b){
            return a;
        }else{
            return b;
        }
    }
}




template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr bool signbit(const fixed_t<Q, D> x){
    constexpr D SIGN_MASK = static_cast<D>(static_cast<D>(1) << size_t(sizeof(D) * 8 - 1));
    return static_cast<bool>(x.to_bits() & SIGN_MASK);
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> sign(const fixed_t<Q, D> x){
    static_assert(fixed_t<Q, D>(-1) != fixed_t<Q, D>(1));
    if(likely(x)) return fixed_t<Q, D>(x > 0 ? 1 : -1);
    else return fixed_t<Q, D>(0);
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> mod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits(a.to_bits() % b.to_bits()));
}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, U> fposmod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    constexpr size_t SHIFT = size_t(sizeof(D) * 8 - 1);
    const D rem = std::bit_cast<D>(a.to_bits() % b.to_bits());
    const D is_negative = static_cast<D>(rem >> SHIFT);  // 符号位扩展（0 或 -1）
    return fixed_t<Q, U>::from_bits(static_cast<U>(rem + static_cast<U>(b.to_bits() & is_negative)));
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> lerp(const fixed_t<Q, D> x, const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return a * (1 - x) + b * x;
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> mean(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits((a.to_bits() + b.to_bits()) >> 1));}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, U> frac(const fixed_t<Q, D> x){
    constexpr U MASK = static_cast<U>((uint64_t(1u) << Q) - 1);
    return fixed_t<Q, U>::from_bits(static_cast<U>(static_cast<U>(x.to_bits()) & MASK));
}


template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr D floor_int(const fixed_t<Q, D> x){
    return static_cast<D>(x.to_bits() >> Q);}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr D ceil_int(const fixed_t<Q, D> x){
    constexpr D MASK = (1 << Q) - 1;
    return static_cast<D>((x.to_bits() >> Q) + bool(x.to_bits() & MASK));
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr D round_int(const fixed_t<Q, D> x){
    constexpr D MASK = (1 << (Q - 1));
    return static_cast<D>((x.to_bits() + MASK) >> Q);
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr T floor_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(floor_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr T ceil_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(ceil_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr T round_cast(const fixed_t<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(round_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> floor(const fixed_t<Q, D> x){
    return floor_int(x);
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> ceil(const fixed_t<Q, D> x){
    return ceil_int(x);
}



template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> round(const fixed_t<Q, D> x){
    return round_int(x);
}



template<size_t Q, typename D>
[[nodiscard]] __attribute__((always_inline))
constexpr fixed_t<Q, D> square(const fixed_t<Q, D> x) {
    return x * x;
}


template<size_t Q, typename D>
static __attribute__((always_inline)) constexpr 
fixed_t<Q, D> errmod(const fixed_t<Q, D> x, const fixed_t<Q, D> s) {
    const auto s_by_2 = s >> 1;
    fixed_t<Q, D> value = mod(x, s);
    if (value > s_by_2) {
        value -= s;
    } else if (value <= -s_by_2) {
        value += s;
    }
    return value;
}

template<size_t Q_to, size_t Q_from, typename D>
static __attribute__((always_inline)) constexpr  
fixed_t<Q_to, int32_t> fixed_downcast(const fixed_t<Q_from, D> val){
    return fixed_t<Q_to, int32_t>::from_bits(static_cast<int32_t>(val.to_bits() >> (Q_from - Q_to)));
}


template<size_t Q>
[[nodiscard]] bool is_equal_approx(
    const fixed_t<Q, int32_t> a, 
    const fixed_t<Q, int32_t> b,
    const fixed_t<Q, int32_t> epsilon
) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a - b == int32_t(0)) {
        return true;
    }
    // Then check for approximate equality.
    fixed_t<Q, int32_t> tolerance = fixed_t<Q, int32_t>() * (a < 0 ? -a : a);
    if (tolerance < fixed_t<Q, int32_t>(epsilon)) {
        tolerance = fixed_t<Q, int32_t>(epsilon);
    }
    return ((a - b < 0) ? b - a : a - b) < tolerance;
}

template<size_t Q>
[[nodiscard]] bool is_equal_approx_ratio(
    const fixed_t<Q, int32_t> a, 
    const fixed_t<Q, int32_t> b, 
    fixed_t<Q, int32_t> epsilon, 
    fixed_t<Q, int32_t> min_epsilon
){

    fixed_t<Q, int32_t> diff = ymd::math::abs(a - b);
    if (diff == 0 || diff < min_epsilon) {
        return true;
    }
    fixed_t<Q, int32_t> avg_size = (ymd::math::abs(a) + ymd::math::abs(b)) >> 1;
    diff = diff / avg_size;
    return diff < epsilon;
}
}
}



namespace std{
    using ymd::fixed_t;
    template<size_t Q, typename D>
    class numeric_limits<fixed_t<Q, D>> {
    public:
        __attribute__((always_inline)) constexpr static fixed_t<Q, D> infinity() noexcept {
            return fixed_t<Q, D>::from_bits(std::numeric_limits<D>::infinity());}
        __attribute__((always_inline)) constexpr static fixed_t<Q, D> lowest() noexcept {
            return fixed_t<Q, D>::from_bits(std::numeric_limits<D>::lowest());}

        __attribute__((always_inline)) constexpr static fixed_t<Q, D> min() noexcept {
            return fixed_t<Q, D>::from_bits(std::numeric_limits<D>::min());}
        __attribute__((always_inline)) constexpr static fixed_t<Q, D> max() noexcept {
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
    __attribute__((always_inline)) constexpr auto signbit(const fixed_t<Q, D> iq)  {
        return ymd::math::signbit(iq);}
    
    template<size_t Q, typename D>
    struct make_signed<fixed_t<Q, D>>{
        using type = fixed_t<Q, D>;
    };

    template<size_t Q, typename D>
    struct make_unsigned<fixed_t<Q, D>>{
        using type = fixed_t<Q, D>;
    };

    template<size_t Q, size_t Q2, typename D>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    fixed_t<Q, D> copysign(const fixed_t<Q, D> x, const fixed_t<Q2, D> s){
        return s > 0 ? x : -x;
    }
}

#include "_literals.ipp"
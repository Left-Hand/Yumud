#pragma once

#include <compare>
#include "core/tmp/integral.hpp"

#include "fxmath/div.hpp"
#include "fxmath/fconv.hpp"

#include <numeric>
#include <cstddef>


#ifndef LOG_E
#define LOG_E (0.434294481903)
#endif

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280)
#endif

#ifndef IQ_DEFAULT_Q
#define IQ_DEFAULT_Q (size_t(16))
#endif


namespace ymd::math{
template<size_t Q, typename D>
struct fixed;
}

namespace std{
    template<size_t Q, typename D>
    struct is_arithmetic<ymd::math::fixed<Q, D>> : std::true_type {};

    template<size_t Q, typename D>
    struct is_floating_point<ymd::math::fixed<Q, D>> : std::false_type {};

    template<size_t Q, typename D>
    struct is_signed<ymd::math::fixed<Q, D>> : std::is_signed<D> {};
}

namespace ymd::tmp{
// 默认模板：非定点数类型
template<typename T>
constexpr bool is_fixed_point_v = false;

// 特化模板：定点数类型
template<size_t Q, typename D>
constexpr bool is_fixed_point_v<math::fixed<Q, D>> = true;


template<typename T>
static constexpr size_t fixed_point_qnum_v = 0;


template<size_t Q, typename D>
static constexpr size_t fixed_point_qnum_v<math::fixed<Q, D>> = Q;

template<typename T>
struct fixed_point_underlying_type{
    using type = void;
};

template<size_t Q, typename D>
struct fixed_point_underlying_type<math::fixed<Q, D>>{
    using type = D;
};

template<typename T>
using fixed_point_underlying_type_t = typename fixed_point_underlying_type<T>::type;
}

namespace ymd::math{


template<size_t Q, typename D>
struct [[nodiscard]] fixed final{
private:
    static_assert(std::is_same_v<D, bool> == false);
    static_assert(std::is_integral_v<D> == true);

    //允许iq0.32的存在 它的值域为[-0.5, 0.5)
    static constexpr size_t MAX_Q = size_t(sizeof(D) * 8);
    
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

    static constexpr size_t NUM_Q = Q;
    using underlying_type = D;

    __attribute__((always_inline)) constexpr fixed(){;}

    __attribute__((always_inline)) constexpr explicit fixed(bits_ctor ctor):
        bits(ctor.count){;}

    [[nodiscard]] static constexpr fixed<Q, D> from_bits(const D bits){
        return fixed<Q, D>(bits_ctor{bits});
    };

    template<typename I>
    requires (
        //必须为非bool的整数类型
        std::is_integral_v<I> and (not std::is_same_v<I, bool>)
    )
    __attribute__((always_inline)) 
    [[nodiscard]] static constexpr fixed<Q, D> from_rcp(const I int_val){
        //如果D是无符号的 I也必须为无符号数
        static_assert(std::is_unsigned_v<D> ? std::is_unsigned_v<I> : true);
        using ED = tmp::extended_underlying_t<D>;
        const auto bits = static_cast<ED>(static_cast<ED>(1u) << Q) / int_val;
        return fixed<Q, D>::from_bits(bits);
    };

    template<size_t P>
    __attribute__((always_inline)) constexpr fixed(const fixed<P, D> & other):
        bits(fixed<P, D>::template transform<Q>(other.to_bits())){};

    template<size_t P>
    __attribute__((always_inline)) constexpr fixed(fixed<P, D> && other):
        bits(fixed<P, D>::template transform<Q>(other.to_bits())){};


    template<size_t P, typename U>
    requires (sizeof(D) == sizeof(U))
    __attribute__((always_inline)) constexpr operator fixed<P, U>() const noexcept {
        const auto new_bits = static_cast<U>(transform<P>(bits));
        return fixed<P, U>::from_bits(new_bits);
    }

    __attribute__((always_inline)) constexpr D to_bits() const noexcept {return bits;}
    
    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed & operator = (const fixed<P, D> & other){
        bits = fixed<P, D>::template transform<Q>(other.to_bits());
        return *this;
    };

    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed & operator = (fixed<P, D> && other){
        bits = fixed<P, D>::template transform<Q>(other.to_bits());
        return *this;
    };
    

    template<typename T>
    requires (std::is_integral_v<T> and (not std::is_same_v<T, bool>))
    __attribute__((always_inline)) constexpr 
    fixed(const T iv):
        fixed([&]{
            if constexpr(sizeof(T) >= 4)
                return bits_ctor{std::bit_cast<D>(static_cast<D>(
                    iv * static_cast<tmp::extended_underlying_t<T>>(uint64_t(1) << Q)
                ))};
            else{
                static_assert(Q <= 32);
                return bits_ctor{std::bit_cast<D>(static_cast<D>(
                    static_cast<int32_t>(iv) * static_cast<int32_t>(uint64_t(1) << Q)
                ))};
            }
        }()
    ){;}

    __attribute__((always_inline)) consteval explicit fixed(const long double dv):
        fixed(bits_ctor{std::bit_cast<D>(static_cast<D>(
            dv * static_cast<long double>(uint64_t(1) << Q)
        ))}){};

    __attribute__((always_inline)) static constexpr 
    fixed from (const std::floating_point auto fv){
        const D bits = [&]() -> D{
            if(std::is_constant_evaluated()){
                // if constexpr((std::is_signed_v<D> == false)){
                //     if(fv < 0) 
                //     __builtin_unreachable();
                // }
                return D(static_cast<long double>(fv) * uint64_t(uint64_t(1) << Q));
            }

            int32_t ret_bits = fxmath::details::_IQFtoN(fv, Q);
            if constexpr((std::is_signed_v<D> == false)){
                if(ret_bits < 0) ret_bits = 0;
            }
            return static_cast<D>(ret_bits);
        }();
        return fixed{bits_ctor{
            bits
        }};
    }

    __attribute__((always_inline)) constexpr 
    fixed operator+() const noexcept {
        return *this;
    }

    __attribute__((always_inline)) constexpr 
    fixed operator-() const 
    requires(std::is_signed_v<D>)
    {
        return fixed::from_bits(-(to_bits()));
    }

    template<typename D2>
    __attribute__((always_inline)) constexpr 
    fixed<Q, D2> cast_inner() const noexcept {
        return fixed<Q, D2>::from_bits(static_cast<D2>(to_bits()));
    }

    //#region addsub
    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed & operator +=(const fixed<P, D> other) {
        return *this = fixed<Q, D>::from_bits(this->to_bits() + fixed<Q, D>(other).to_bits());
    }

    __attribute__((always_inline)) constexpr 
    fixed & operator +=(const std::integral auto other) {
        return *this += fixed<Q, D>(other);
    }


    template<size_t P>
    __attribute__((always_inline)) constexpr 
    fixed & operator -=(const fixed<P, D> other) {
        return *this = fixed<Q, D>::from_bits(this->to_bits() - fixed<Q, D>(other).to_bits());
    }

    __attribute__((always_inline)) constexpr 
    fixed & operator -=(const std::integral auto other) {
        return *this -= fixed<Q, D>(other);
    }
    //#endregion addsub

    //#region shifts
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    fixed operator<<(size_t shift) const noexcept {
        return fixed::from_bits((this->to_bits() << shift));
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    fixed operator>>(size_t shift) const noexcept {
        return fixed::from_bits((this->to_bits() >> shift));
    }
    //#endregion

    [[nodiscard]] __attribute__((always_inline)) constexpr explicit 
    operator bool() const noexcept {
        return bool(this->to_bits());
    }

    template<typename T>
    requires std::is_integral_v<T>
    [[nodiscard]] __attribute__((always_inline)) constexpr explicit 
    operator T() const noexcept {
        return static_cast<T>(this->to_bits() >> Q);
    }
    

    template<typename T>
    requires std::is_floating_point_v<T> __inline constexpr explicit 
    operator T() const noexcept {
        if(std::is_constant_evaluated()){
            return static_cast<long double>(this->to_bits()) / static_cast<long double>(uint64_t(1u) << Q);
        }else{
            if constexpr(std::is_signed_v<D>){
                static_assert(sizeof(D) <= 4);
                return fxmath::details::_IQNtoF(this->to_bits(), Q);
            }else{
                static_assert(sizeof(D) <= 4);
                return fxmath::details::_IQNtoF(
                    (this->to_bits() + 1) >> 1u, 
                    Q - 1);
            }
        }
    }
};

template<size_t Q, size_t P, 
    typename D1, typename D2, 
    typename D = tmp::sum_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator +(const fixed<Q, D1> lhs, const fixed<P, D2> rhs) {
    fixed<Q, D> ret = fixed<Q, D>(rhs);
    ret += fixed<Q, D>(lhs);
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator +(const std::integral auto lhs, const fixed<Q, D> rhs) {
    fixed<Q, D> ret = fixed<Q, D>(lhs);
    ret += rhs;
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator +(const fixed<Q, D> lhs, const std::integral auto rhs) {
    fixed<Q, D> ret = lhs;
    ret += fixed<Q, D>(rhs);
    return ret;
}

template<size_t Q, size_t P, 
    typename D1, typename D2, 
    typename D = tmp::sum_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator -(const fixed<Q, D1> lhs, const fixed<P, D2> rhs) {
    fixed<Q, D> ret = fixed<Q, D>(lhs);
    ret -= fixed<Q, D>(rhs);
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator -(const std::integral auto lhs, const fixed<Q, D> rhs) {
    fixed<Q, D> ret = fixed<Q, D>(lhs);
    ret -= rhs;
    return ret;
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator -(const fixed<Q, D> lhs, const std::integral auto rhs) {
    fixed<Q, D> ret = lhs;
    ret -= fixed<Q, D>(rhs);
    return ret;
}


template<size_t Q1, size_t Q2, 
    typename D1, typename D2, 
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed<Q1, D> operator *(const fixed<Q1, D1> lhs, const fixed<Q2, D2> rhs) {
    using ED = tmp::extended_mul_underlying_t<D1, D2>;
    return fixed<Q1, D>::from_bits(
        static_cast<D>(static_cast<ED>(lhs.to_bits()) * static_cast<ED>(rhs.to_bits()) >> Q2)
    );
}

template<size_t Q, 
    typename D1, typename D2, 
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator *(const D2 lhs, const fixed<Q, D1> rhs) {
    return fixed<Q, D>::from_bits(
        static_cast<D>(lhs) * static_cast<D>(rhs.to_bits())
    );
}

template<size_t Q, 
    typename D1, typename D2, 
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator *(const fixed<Q, D1> lhs, const D2 rhs) {
    return fixed<Q, D>::from_bits(
        static_cast<D>(lhs.to_bits()) * static_cast<D>(rhs)
    );
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator *(const bool lhs, const fixed<Q, D> rhs) {
    return lhs ? rhs : fixed<Q, D>::from_bits(0);
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator *(const fixed<Q, D> lhs, const bool rhs) {
    return rhs ? lhs : fixed<Q, D>::from_bits(0);
}

template<
    size_t Q, size_t P, 
    typename D1, typename D2,
    typename D = tmp::mul_underlying_t<D1, D2>
>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator /(const fixed<Q, D1> lhs, const fixed<P, D2> rhs) {
    static_assert(sizeof(D1) == sizeof(D2));
    if (std::is_constant_evaluated()) {
        return fixed<Q, D>::from(static_cast<long double>(lhs) / static_cast<long double>(rhs));
    }else{
        constexpr bool result_is_signed = std::is_signed_v<D1> or std::is_signed_v<D2>;
        static_assert(sizeof(D) == 4);
        if constexpr(result_is_signed){
            return fixed<Q, D>::from_bits(fxmath::details::div32i<Q>(
                lhs.to_bits(), rhs.to_bits()
            ));
        }else{
            return fixed<Q, D>::from_bits(fxmath::details::div32u<Q>(
                lhs.to_bits(), rhs.to_bits()
            ));
        }
    }
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto operator /(const std::integral auto lhs, const fixed<Q, D> rhs) {
    constexpr bool result_is_signed = std::is_signed_v<D> or std::is_signed_v<decltype(lhs)>;
    using result_underly_t = std::conditional_t<result_is_signed, std::make_signed_t<D>, D>;
	return fixed<Q, result_underly_t>(lhs) / rhs;
}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed<Q, D> operator /(const fixed<Q, D> lhs, const std::integral auto rhs) {
    // cpp的标准规定有符号数除以无符号数会先将被除数转为无符号数 导致发生诸如-30 / 2被转换为了非常大的整数
    // 这里的做法是将除数转为有符号数
    if constexpr(std::is_signed_v<D> and std::is_unsigned_v<decltype(rhs)>){
        using signed_rhs_t = std::make_signed_t<std::decay_t<decltype(rhs)>>;
        return fixed<Q, D>::from_bits(lhs.to_bits() / static_cast<signed_rhs_t>(rhs));
    }else{
        return fixed<Q, D>::from_bits(lhs.to_bits() / rhs);
    }
}




template<size_t Q1, size_t Q2, typename D1, typename D2>
[[nodiscard]] __attribute__((always_inline)) constexpr 
std::strong_ordering operator <=> (const fixed<Q1, D1> & self, const fixed<Q2, D2> & other) {
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


template<size_t Q, typename D, std::floating_point T>
[[nodiscard]] __attribute__((always_inline)) consteval 
std::strong_ordering operator <=> (const fixed<Q, D> & self, const T & other) {
    return (std::bit_cast<D>(self.to_bits()) <=> fixed<Q, D>(other));
}


template<size_t Q, typename D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
std::strong_ordering operator <=> (const fixed<Q, D> & self, const T & other) {
    return (std::bit_cast<D>(self.to_bits()) <=> (D(other) << Q));
}


template<size_t Q, typename D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
std::strong_ordering operator <=> (const T & other, const fixed<Q,D> & self){
    return (std::bit_cast<D>(self.to_bits()) <=> (D(other) << Q));
}

// 统一的等于运算符模板，直接复用 <=>
template<size_t Q, typename D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool operator == (const fixed<Q, D> & self, const T & other) {
    return (self <=> other) == 0;
}

template<size_t Q, typename D, std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool operator == (const T & other, const fixed<Q, D> & self) {
    return (self <=> other) == 0;  // 注意这里复用 self <=> other
}


template<size_t Q1, size_t Q2, typename D1, typename D2>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool operator == (const fixed<Q1, D1> & self, const fixed<Q2, D2> & other) {
    return (self <=> other) == 0;  // 注意这里复用 self <=> other
}

#if 0
template<size_t Q, typename D, typename U = std::make_unsigned_t<D>> static [[n
    odiscard]] __attribute__((const, always_inline))
constexpr fixed<Q, U> abs(const fixed<Q, D> x){
    const auto bits = x.to_bits();
    return fixed<Q, U>::from_bits(static_cast<U>(bits > 0 ? bits : -bits));
}
#else
template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> abs(const fixed<Q, D> x){
    const auto bits = x.to_bits();
    return fixed<Q, D>::from_bits(static_cast<D>(bits > 0 ? bits : -bits));
}
#endif

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
bool signbit(const fixed<Q, D> x){
    constexpr D SIGN_MASK = static_cast<D>(static_cast<D>(1) << size_t(sizeof(D) * 8 - 1));
    return static_cast<bool>(x.to_bits() & SIGN_MASK);
}

}




namespace std{

    template<size_t Q, typename D>
    class numeric_limits<ymd::math::fixed<Q, D>> {
    public:
        __attribute__((always_inline)) constexpr static ymd::math::fixed<Q, D> infinity() noexcept {
            return ymd::math::fixed<Q, D>::from_bits(std::numeric_limits<D>::infinity());}
        __attribute__((always_inline)) constexpr static ymd::math::fixed<Q, D> lowest() noexcept {
            return ymd::math::fixed<Q, D>::from_bits(std::numeric_limits<D>::lowest());}

        __attribute__((always_inline)) constexpr static ymd::math::fixed<Q, D> min() noexcept {
            return ymd::math::fixed<Q, D>::from_bits(std::numeric_limits<D>::min());}
        __attribute__((always_inline)) constexpr static ymd::math::fixed<Q, D> max() noexcept {
            return ymd::math::fixed<Q, D>::from_bits(std::numeric_limits<D>::max());}
    };

    template<size_t Q, typename D>
    struct common_type<ymd::math::fixed<Q, D>, float> {
        using type = ymd::math::fixed<Q, D>;
    };

    template<size_t Q, typename D>
    struct common_type<ymd::math::fixed<Q, D>, double> {
        using type = ymd::math::fixed<Q, D>;
    };

    template<size_t Q, typename D>
    struct common_type<float, ymd::math::fixed<Q, D>> {
        using type = ymd::math::fixed<Q, D>;
    };

    template<size_t Q, typename D>
    struct common_type<double, ymd::math::fixed<Q, D>> {
        using type = ymd::math::fixed<Q, D>;
    };

    template<size_t Q, typename D>
    [[nodiscard]] __attribute__((always_inline)) constexpr bool 
    signbit(const ymd::math::fixed<Q, D> x)  {
        constexpr D SIGN_MASK = static_cast<D>(static_cast<D>(1) << size_t(sizeof(D) * 8 - 1));
        return static_cast<bool>(x.to_bits() & SIGN_MASK);
    }
    
    template<size_t Q, typename D>
    struct make_signed<ymd::math::fixed<Q, D>>{
        using type = ymd::math::fixed<Q, D>;
    };

    template<size_t Q, typename D>
    struct make_unsigned<ymd::math::fixed<Q, D>>{
        using type = ymd::math::fixed<Q, D>;
    };

    template<size_t Q, size_t Q2, typename D>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    ymd::math::fixed<Q, D> copysign(const ymd::math::fixed<Q, D> x, const ymd::math::fixed<Q2, D> s){
        return s > 0 ? x : -x;
    }
}

#include "_literals.ipp"
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
}

namespace ymd{
// 默认模板：非定点数类型
template<typename T>
constexpr bool is_fixed_point_v = false;

// 特化模板：定点数类型
template<size_t Q, typename D>
constexpr bool is_fixed_point_v<fixed_t<Q, D>> = true;

template<size_t Q, typename D>
struct fixed_t{
private:
    static_assert(Q < sizeof(D) * 8);

    D count_;
public:
    struct count_ctor{
        D count;
    };

    template<size_t P>
    [[nodiscard]] __fast_inline static constexpr D transform(const D value){
        if constexpr (P > Q){
            return D(D(value) << (P - Q));
        }else if constexpr (P < Q){
            return D(D(value) >> (Q - P));
        }else{
            return D(D(value));
        }
    }

    static constexpr size_t q_num = Q;

    __fast_inline constexpr fixed_t(){;}

    __fast_inline constexpr fixed_t(count_ctor ctor):
        count_(ctor.count){;}

    template<size_t P>
    __fast_inline constexpr operator fixed_t<P, D>() const {
        return fixed_t<P, D>::from_bits(transform<P>(count_));}
    
    __fast_inline constexpr fixed_t(const fixed_t<Q, D> & other):
        count_(other.count_){};

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
        fixed_t(count_ctor{static_cast<D>(iv << Q)}){;}

    __fast_inline consteval explicit fixed_t(const long double dv):
        fixed_t(count_ctor{static_cast<D>(dv * static_cast<long double>(1u << Q))}){};

    static __fast_inline constexpr fixed_t from (const floating auto fv){
        return fixed_t{
            count_ctor{iqmath::details::_IQFtoN<Q>(fv)}
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
            (int64_t(this->as_bits()) * int64_t((other).as_bits())) >> (P)
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
    __fast_inline constexpr bool operator op (const fixed_t<P, D> other) const {\
        return as_bits() op (fixed_t<Q, D>(other).as_bits());\
    }\
    \
    template<typename T>\
    requires std::is_floating_point_v<T>\
    __fast_inline constexpr bool operator op (const T other) const {\
        return (*this op fixed_t<Q, D>(other));\
    }\
    \
    template<typename T>\
    requires std::is_integral_v<T>\
    __fast_inline constexpr bool operator op (const T other) const {\
        return (((as_bits())) op (D(other) << Q));\
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
    __fast_inline constexpr fixed_t operator<<(D shift) const {
        return fixed_t::from_bits((this->as_bits() << shift));
    }

    __fast_inline constexpr fixed_t operator>>(D shift) const {
        return fixed_t::from_bits((this->as_bits() >> shift));
    }
    //#endregion

    __fast_inline constexpr explicit operator bool() const {
        return bool(this->as_bits());
    }

    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr explicit operator T() const {
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

template<typename T>
concept convertible_to_iq_v = std::is_convertible_v<T, fixed_t<16, int32_t>>;

template<typename T>
concept cast_to_iq_v = (std::is_convertible_v<T, fixed_t<16, int32_t>> && (!is_fixed_point_v<T>));

template<size_t Q, typename D>
__fast_inline constexpr bool operator >(const cast_to_iq_v auto lhs, const fixed_t<Q, D> rhs) {
	return fixed_t<Q, D>(lhs) > rhs;
}

template<size_t Q, typename D>
__fast_inline constexpr bool operator >=(const cast_to_iq_v auto lhs, const fixed_t<Q, D> rhs) {
	return fixed_t<Q, D>(lhs) > rhs;
}

template<size_t Q, typename D>
__fast_inline constexpr bool operator <(const cast_to_iq_v auto lhs, const fixed_t<Q, D> rhs) {
	return fixed_t<Q, D>(lhs) > rhs;
}

template<size_t Q, typename D>
__fast_inline constexpr bool operator <=(const cast_to_iq_v auto lhs, const fixed_t<Q, D> rhs) {
	return fixed_t<Q, D>(lhs) > rhs;
}

template<size_t Q, typename D>
__fast_inline constexpr bool operator ==(const cast_to_iq_v auto lhs, const fixed_t<Q, D> rhs) {
	return fixed_t<Q, D>(lhs) > rhs;
}

template<size_t Q, typename D>
__fast_inline constexpr bool operator !=(const cast_to_iq_v auto lhs, const fixed_t<Q, D> rhs) {
	return fixed_t<Q, D>(lhs) > rhs;
}


consteval fixed_t<1, int32_t> operator"" _q1(long double x) { return fixed_t<1, int32_t>(x); }
consteval fixed_t<2, int32_t> operator"" _q2(long double x) { return fixed_t<2, int32_t>(x); }
consteval fixed_t<3, int32_t> operator"" _q3(long double x) { return fixed_t<3, int32_t>(x); }
consteval fixed_t<4, int32_t> operator"" _q4(long double x) { return fixed_t<4, int32_t>(x); }
consteval fixed_t<5, int32_t> operator"" _q5(long double x) { return fixed_t<5, int32_t>(x); }
consteval fixed_t<6, int32_t> operator"" _q6(long double x) { return fixed_t<6, int32_t>(x); }
consteval fixed_t<7, int32_t> operator"" _q7(long double x) { return fixed_t<7, int32_t>(x); }
consteval fixed_t<8, int32_t> operator"" _q8(long double x) { return fixed_t<8, int32_t>(x); }
consteval fixed_t<9, int32_t> operator"" _q9(long double x) { return fixed_t<9, int32_t>(x); }
consteval fixed_t<10, int32_t> operator"" _q10(long double x) { return fixed_t<10, int32_t>(x); }
consteval fixed_t<11, int32_t> operator"" _q11(long double x) { return fixed_t<11, int32_t>(x); }
consteval fixed_t<12, int32_t> operator"" _q12(long double x) { return fixed_t<12, int32_t>(x); }
consteval fixed_t<13, int32_t> operator"" _q13(long double x) { return fixed_t<13, int32_t>(x); }
consteval fixed_t<14, int32_t> operator"" _q14(long double x) { return fixed_t<14, int32_t>(x); }
consteval fixed_t<15, int32_t> operator"" _q15(long double x) { return fixed_t<15, int32_t>(x); }
consteval fixed_t<16, int32_t> operator"" _q16(long double x) { return fixed_t<16, int32_t>(x); }
consteval fixed_t<17, int32_t> operator"" _q17(long double x) { return fixed_t<17, int32_t>(x); }
consteval fixed_t<18, int32_t> operator"" _q18(long double x) { return fixed_t<18, int32_t>(x); }
consteval fixed_t<19, int32_t> operator"" _q19(long double x) { return fixed_t<19, int32_t>(x); }
consteval fixed_t<20, int32_t> operator"" _q20(long double x) { return fixed_t<20, int32_t>(x); }
consteval fixed_t<21, int32_t> operator"" _q21(long double x) { return fixed_t<21, int32_t>(x); }
consteval fixed_t<22, int32_t> operator"" _q22(long double x) { return fixed_t<22, int32_t>(x); }
consteval fixed_t<23, int32_t> operator"" _q23(long double x) { return fixed_t<23, int32_t>(x); }
consteval fixed_t<24, int32_t> operator"" _q24(long double x) { return fixed_t<24, int32_t>(x); }
consteval fixed_t<25, int32_t> operator"" _q25(long double x) { return fixed_t<25, int32_t>(x); }
consteval fixed_t<26, int32_t> operator"" _q26(long double x) { return fixed_t<26, int32_t>(x); }
consteval fixed_t<27, int32_t> operator"" _q27(long double x) { return fixed_t<27, int32_t>(x); }
consteval fixed_t<28, int32_t> operator"" _q28(long double x) { return fixed_t<28, int32_t>(x); }
consteval fixed_t<29, int32_t> operator"" _q29(long double x) { return fixed_t<29, int32_t>(x); }
consteval fixed_t<30, int32_t> operator"" _q30(long double x) { return fixed_t<30, int32_t>(x); }
consteval fixed_t<31, int32_t> operator"" _q31(long double x) { return fixed_t<31, int32_t>(x); }

consteval fixed_t<1, int32_t> operator"" _q1(unsigned long long x) { return fixed_t<1, int32_t>(x); }
consteval fixed_t<2, int32_t> operator"" _q2(unsigned long long x) { return fixed_t<2, int32_t>(x); }
consteval fixed_t<3, int32_t> operator"" _q3(unsigned long long x) { return fixed_t<3, int32_t>(x); }
consteval fixed_t<4, int32_t> operator"" _q4(unsigned long long x) { return fixed_t<4, int32_t>(x); }
consteval fixed_t<5, int32_t> operator"" _q5(unsigned long long x) { return fixed_t<5, int32_t>(x); }
consteval fixed_t<6, int32_t> operator"" _q6(unsigned long long x) { return fixed_t<6, int32_t>(x); }
consteval fixed_t<7, int32_t> operator"" _q7(unsigned long long x) { return fixed_t<7, int32_t>(x); }
consteval fixed_t<8, int32_t> operator"" _q8(unsigned long long x) { return fixed_t<8, int32_t>(x); }
consteval fixed_t<9, int32_t> operator"" _q9(unsigned long long x) { return fixed_t<9, int32_t>(x); }
consteval fixed_t<10, int32_t> operator"" _q10(unsigned long long x) { return fixed_t<10, int32_t>(x); }
consteval fixed_t<11, int32_t> operator"" _q11(unsigned long long x) { return fixed_t<11, int32_t>(x); }
consteval fixed_t<12, int32_t> operator"" _q12(unsigned long long x) { return fixed_t<12, int32_t>(x); }
consteval fixed_t<13, int32_t> operator"" _q13(unsigned long long x) { return fixed_t<13, int32_t>(x); }
consteval fixed_t<14, int32_t> operator"" _q14(unsigned long long x) { return fixed_t<14, int32_t>(x); }
consteval fixed_t<15, int32_t> operator"" _q15(unsigned long long x) { return fixed_t<15, int32_t>(x); }
consteval fixed_t<16, int32_t> operator"" _q16(unsigned long long x) { return fixed_t<16, int32_t>(x); }
consteval fixed_t<17, int32_t> operator"" _q17(unsigned long long x) { return fixed_t<17, int32_t>(x); }
consteval fixed_t<18, int32_t> operator"" _q18(unsigned long long x) { return fixed_t<18, int32_t>(x); }
consteval fixed_t<19, int32_t> operator"" _q19(unsigned long long x) { return fixed_t<19, int32_t>(x); }
consteval fixed_t<20, int32_t> operator"" _q20(unsigned long long x) { return fixed_t<20, int32_t>(x); }
consteval fixed_t<21, int32_t> operator"" _q21(unsigned long long x) { return fixed_t<21, int32_t>(x); }
consteval fixed_t<22, int32_t> operator"" _q22(unsigned long long x) { return fixed_t<22, int32_t>(x); }
consteval fixed_t<23, int32_t> operator"" _q23(unsigned long long x) { return fixed_t<23, int32_t>(x); }
consteval fixed_t<24, int32_t> operator"" _q24(unsigned long long x) { return fixed_t<24, int32_t>(x); }
consteval fixed_t<25, int32_t> operator"" _q25(unsigned long long x) { return fixed_t<25, int32_t>(x); }
consteval fixed_t<26, int32_t> operator"" _q26(unsigned long long x) { return fixed_t<26, int32_t>(x); }
consteval fixed_t<27, int32_t> operator"" _q27(unsigned long long x) { return fixed_t<27, int32_t>(x); }
consteval fixed_t<28, int32_t> operator"" _q28(unsigned long long x) { return fixed_t<28, int32_t>(x); }
consteval fixed_t<29, int32_t> operator"" _q29(unsigned long long x) { return fixed_t<29, int32_t>(x); }
consteval fixed_t<30, int32_t> operator"" _q30(unsigned long long x) { return fixed_t<30, int32_t>(x); }
consteval fixed_t<31, int32_t> operator"" _q31(unsigned long long x) { return fixed_t<31, int32_t>(x); }

using q1 = fixed_t<1, int32_t>;
using q2 = fixed_t<2, int32_t>;
using q3 = fixed_t<3, int32_t>;
using q4 = fixed_t<4, int32_t>;
using q5 = fixed_t<5, int32_t>;
using q6 = fixed_t<6, int32_t>;
using q7 = fixed_t<7, int32_t>;
using q8 = fixed_t<8, int32_t>;
using q9 = fixed_t<9, int32_t>;
using q10 = fixed_t<10, int32_t>;
using q11 = fixed_t<11, int32_t>;
using q12 = fixed_t<12, int32_t>;
using q13 = fixed_t<13, int32_t>;
using q14 = fixed_t<14, int32_t>;
using q15 = fixed_t<15, int32_t>;
using q16 = fixed_t<16, int32_t>;
using q17 = fixed_t<17, int32_t>;
using q18 = fixed_t<18, int32_t>;
using q19 = fixed_t<19, int32_t>;
using q20 = fixed_t<20, int32_t>;
using q21 = fixed_t<21, int32_t>;
using q22 = fixed_t<22, int32_t>;
using q23 = fixed_t<23, int32_t>;
using q24 = fixed_t<24, int32_t>;
using q25 = fixed_t<25, int32_t>;
using q26 = fixed_t<26, int32_t>;
using q27 = fixed_t<27, int32_t>;
using q28 = fixed_t<28, int32_t>;
using q29 = fixed_t<29, int32_t>;
using q30 = fixed_t<30, int32_t>;
using q31 = fixed_t<31, int32_t>;

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> abs(const fixed_t<Q, D> x){
    const auto ivalue = x.as_bits();
    return fixed_t<Q, D>::from_bits(ivalue > 0 ? ivalue : -ivalue);
}

template<size_t Q, typename D>
__fast_inline constexpr bool signbit(const fixed_t<Q, D> x){
    return x.as_bits() & static_cast<D>(static_cast<D>(1) << size_t(sizeof(D) * 8 - 1));
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

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> fposmod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    const D mod_result = a.as_bits() % b.as_bits();
    const D is_negative = static_cast<D>(mod_result >> size_t(sizeof(D) * 8 - 1));  // 符号位扩展（0 或 -1）
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits(mod_result + (b.as_bits() & is_negative)));
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> lerp(const fixed_t<Q, D> x, const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return a * (1 - x) + b * x;
}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> mean(const fixed_t<Q, D> a, const fixed_t<Q, D> b){
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits((a.as_bits() + b.as_bits()) >> 1));}

template<size_t Q, typename D>
__fast_inline constexpr fixed_t<Q, D> frac(const fixed_t<Q, D> x){
    static constexpr D MASK = ((1 << Q) - 1);
    return fixed_t<Q, D>(fixed_t<Q, D>::from_bits((x.as_bits()) & MASK));
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
bool not_in_one(const fixed_t<Q, D> qv){
    // return (qv.as_bits() & (~uint32_t((1u << Q) - 1)));
    if(qv < fixed_t<Q, D>(-0.001)) return true;
    if(qv > fixed_t<Q, D>(1.001)) return true;
    return false;
}

template<size_t Q, typename D>
bool is_in_one(const fixed_t<Q, D> qv){
    return not is_in_one(qv);
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
        __fast_inline constexpr static fixed_t<Q, D> infinity() noexcept {return fixed_t<Q, D>::from_bits(0x7FFFFFFF);}
        __fast_inline constexpr static fixed_t<Q, D> lowest() noexcept {return fixed_t<Q, D>::from_bits(0x80000000);}

        __fast_inline constexpr static fixed_t<Q, D> min() noexcept {return fixed_t<Q, D>::from_bits(0x80000000);}
        __fast_inline constexpr static fixed_t<Q, D> max() noexcept {return fixed_t<Q, D>::from_bits(0x7FFFFFFF);}
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
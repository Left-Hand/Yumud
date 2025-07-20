#pragma once


#include "core/platform.hpp"

#include "universal/support.hpp"
#include "universal/_IQNtoF.hpp"
#include "universal/_IQFtoN.hpp"
#include "universal/_IQNdiv.hpp"


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
template<size_t Q>
struct iq_t;
}

namespace std{
    template<size_t Q>
    requires (Q < 32)
    struct is_arithmetic<ymd::iq_t<Q>> : std::true_type {};

    template<size_t Q>
    requires (Q < 32)
    struct is_floating_point<ymd::iq_t<Q>> : std::false_type {};
}

namespace ymd{
// 默认模板：非定点数类型
template<typename T>
constexpr bool is_fixed_point_v = false;

// 特化模板：定点数类型
template<size_t Q>
requires (Q < 32)
constexpr bool is_fixed_point_v<iq_t<Q>> = true;

template<size_t Q>
struct iq_t{
public:
    using iq_type = _iq<Q>;
private:
    static_assert(Q < 32, "Q must be less than 32");

    iq_type value_;
public:

    static constexpr size_t q_num = Q;


    __fast_inline constexpr iq_t(){;}

    template<size_t P>
    __fast_inline constexpr iq_t(const _iq<P> otherv) : value_(otherv){;}

    __fast_inline explicit constexpr operator _iq<Q>() const {return value_;}

    template<size_t P>
    __fast_inline constexpr operator iq_t<P>() const {return iq_t<P>(value_);}
    
    __fast_inline constexpr iq_t(const iq_t<Q> & other):value_(other.qvalue()){};
    
    template<size_t P>
    __fast_inline constexpr iq_t & operator = (const iq_t<P> & other){
        value_ = _iq<Q>(other.qvalue());
        return *this;
    };
    

    template<size_t P>
    __fast_inline constexpr iq_t & operator = (iq_t<P> && other){
        value_ = _iq<Q>(other.qvalue());
        return *this;
    };
    
    static constexpr iq_t<Q> from_i32(const int32_t value_){
        return iq_t<Q>(_iq<Q>::from_i32(value_));
    };

    constexpr int32_t as_i32() const{
        return value_.as_i32();
    };

    constexpr _iq<Q> qvalue() const {
        return value_;  
    }

    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr iq_t(const T iv):
        value_(_iq<Q>::from_i32(iv << Q)) {;}

    __fast_inline consteval explicit iq_t(const long double dv):
        value_(_iq<Q>::from_i32(dv * static_cast<long double>(1u << Q))){};

    static __fast_inline constexpr iq_t from (const floating auto fv){
        return iq_t{__iqdetails::_IQFtoN<Q>(fv)};}

    __fast_inline constexpr iq_t operator+() const {
        return *this;
    }

    __fast_inline constexpr iq_t operator-() const {
        return iq_t(_iq<Q>::from_i32(-(as_i32())));
    }

    //#region addsub
    template<size_t P>
    __fast_inline constexpr iq_t & operator +=(const iq_t<P> other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(this->as_i32() + _iq<Q>(other.qvalue()).as_i32()));
    }

    __fast_inline constexpr iq_t & operator +=(const integral auto other) {
        return *this += iq_t<Q>(other);
    }

    template<size_t P>
    __fast_inline constexpr iq_t & operator -=(const iq_t<P> other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(this->as_i32() - _iq<Q>(other.qvalue()).as_i32()));
    }

    __fast_inline constexpr iq_t & operator -=(const integral auto other) {
        return *this -= iq_t<Q>(other);
    }
    //#endregion addsub

    //#region multiply

    template<size_t P>
    __fast_inline constexpr iq_t& operator *=(const iq_t<P> other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(
            (int64_t(this->as_i32()) * int64_t((other).as_i32())) >> (P)
        ));
    }

    __fast_inline constexpr iq_t& operator *=(const integral auto other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(this->as_i32() * other));
    }
    
    //#endregion

    //#region division
    __fast_inline constexpr iq_t & operator/=(const integral auto other) {
        return *this = iq_t(_iq<Q>::from_i32((as_i32() / other)));
    }

    template<size_t P>
    __fast_inline constexpr iq_t & operator/=(const iq_t<P> other) {
        if (std::is_constant_evaluated()) {
            return *this = iq_t<Q>::from(float(*this) / float(other));
        }else{
            return *this = iq_t(__iqdetails::_IQNdiv<Q>(value_, _iq<Q>(other.qvalue())));
        }
    }

    //#endregion

    //#region comparisons
    #define IQ_COMP_TEMPLATE(op)\
    template<size_t P>\
    __fast_inline constexpr bool operator op (const iq_t<P> other) const {\
        return as_i32() op iq_type(other.qvalue()).as_i32();\
    }\
    \
    template<typename T>\
    requires std::is_floating_point_v<T>\
    __fast_inline constexpr bool operator op (const T other) const {\
        return (*this op iq_t<Q>(other));\
    }\
    \
    template<typename T>\
    requires std::is_integral_v<T>\
    __fast_inline constexpr bool operator op (const T other) const {\
        return (((as_i32())) op (int32_t(other) << Q));\
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
    __fast_inline constexpr iq_t operator<<(int shift) const {
        return iq_t(_iq<Q>::from_i32(as_i32() << shift));
    }

    __fast_inline constexpr iq_t operator>>(int shift) const {
        return iq_t(_iq<Q>::from_i32(as_i32() >> shift));
    }
    //#endregion

    __fast_inline constexpr explicit operator bool() const {
        return bool(as_i32());
    }

    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr explicit operator T() const {
        return as_i32() >> Q;
    }
    

    template<typename T>
    requires std::is_floating_point_v<T>
    __inline constexpr explicit operator T() const{
        if(std::is_constant_evaluated()){
            return float(as_i32()) / int(1u << Q);
        }else{
            return __iqdetails::_IQNtoF<Q>(value_);
        }
    }
};

template<size_t Q, size_t P>
__fast_inline constexpr iq_t<Q> operator +(const iq_t<Q> iq_v, const iq_t<P> val) {
    iq_t<Q> ret = iq_t<Q>(val);
    ret += iq_v;
    return ret;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator +(const integral auto val, const iq_t<Q> iq_v) {
    iq_t<Q> ret = iq_t<Q>(val);
    ret += iq_v;
    return ret;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator +(const iq_t<Q> iq_v, const integral auto val) {
    iq_t<Q> ret = iq_v;
    ret += iq_t<Q>(val);
    return ret;
}

template<size_t Q, size_t P>
__fast_inline constexpr iq_t<Q> operator -(const iq_t<Q> iq_v, const iq_t<P> val) {
    iq_t<Q> ret = iq_t<Q>(iq_v);
    ret -= val;
    return ret;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator -(const integral auto val, const iq_t<Q> iq_v) {
    iq_t<Q> ret = iq_t<Q>(val);
    ret -= iq_v;
    return ret;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator -(const iq_t<Q> iq_v, const integral auto val) {
    iq_t<Q> ret = iq_v;
    ret -= iq_t<Q>(val);
    return ret;
}


template<size_t Q, size_t P>
__fast_inline constexpr iq_t<Q> operator *(const iq_t<Q> val, const iq_t<P> iq_v) {
    iq_t<Q> ret = val;
    ret *= iq_v;
    return ret;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator *(const integral auto val, const iq_t<Q> iq_v) {
    iq_t<Q> ret = iq_v;
    ret *= val;
    return ret;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator *(const iq_t<Q> iq_v, const integral auto val) {
    iq_t<Q> ret = iq_v;
    ret *= val;
    return ret;
}

template<size_t Q, size_t P>
__fast_inline constexpr iq_t<Q> operator /(const iq_t<Q> val, const iq_t<P> iq_v) {
    iq_t<Q> ret = val;
    ret /= iq_v;
    return ret;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator /(const integral auto val, const iq_t<Q> iq_v) {
	iq_t<Q> ret = iq_t<Q>(val);
    ret /= iq_v;
    return ret;
}


template<size_t Q>
__fast_inline constexpr iq_t<Q> operator /(const iq_t<Q> iq_v, const integral auto val) {
	iq_t<Q> ret = iq_t<Q>(iq_v);
    ret /= val;
    return ret;
}

template<typename T>
concept convertible_to_iq_v = std::is_convertible_v<T, iq_t<16>>;

template<typename T>
concept cast_to_iq_v = (std::is_convertible_v<T, iq_t<16>> && (!is_fixed_point_v<T>));

template<size_t Q>
__fast_inline constexpr bool operator >(const cast_to_iq_v auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) > iq_v;
}

template<size_t Q>
__fast_inline constexpr bool operator >=(const cast_to_iq_v auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) > iq_v;
}

template<size_t Q>
__fast_inline constexpr bool operator <(const cast_to_iq_v auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) > iq_v;
}

template<size_t Q>
__fast_inline constexpr bool operator <=(const cast_to_iq_v auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) > iq_v;
}

template<size_t Q>
__fast_inline constexpr bool operator ==(const cast_to_iq_v auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) > iq_v;
}

template<size_t Q>
__fast_inline constexpr bool operator !=(const cast_to_iq_v auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) > iq_v;
}



consteval iq_t<IQ_DEFAULT_Q> operator"" _q(long double x){
    return iq_t<IQ_DEFAULT_Q>(x);
}

consteval iq_t<IQ_DEFAULT_Q> operator"" _q(unsigned long long x){
    return iq_t<IQ_DEFAULT_Q>(x);
}

consteval iq_t<1> operator"" _q1(long double x) { return iq_t<1>(x); }
consteval iq_t<2> operator"" _q2(long double x) { return iq_t<2>(x); }
consteval iq_t<3> operator"" _q3(long double x) { return iq_t<3>(x); }
consteval iq_t<4> operator"" _q4(long double x) { return iq_t<4>(x); }
consteval iq_t<5> operator"" _q5(long double x) { return iq_t<5>(x); }
consteval iq_t<6> operator"" _q6(long double x) { return iq_t<6>(x); }
consteval iq_t<7> operator"" _q7(long double x) { return iq_t<7>(x); }
consteval iq_t<8> operator"" _q8(long double x) { return iq_t<8>(x); }
consteval iq_t<9> operator"" _q9(long double x) { return iq_t<9>(x); }
consteval iq_t<10> operator"" _q10(long double x) { return iq_t<10>(x); }
consteval iq_t<11> operator"" _q11(long double x) { return iq_t<11>(x); }
consteval iq_t<12> operator"" _q12(long double x) { return iq_t<12>(x); }
consteval iq_t<13> operator"" _q13(long double x) { return iq_t<13>(x); }
consteval iq_t<14> operator"" _q14(long double x) { return iq_t<14>(x); }
consteval iq_t<15> operator"" _q15(long double x) { return iq_t<15>(x); }
consteval iq_t<16> operator"" _q16(long double x) { return iq_t<16>(x); }
consteval iq_t<17> operator"" _q17(long double x) { return iq_t<17>(x); }
consteval iq_t<18> operator"" _q18(long double x) { return iq_t<18>(x); }
consteval iq_t<19> operator"" _q19(long double x) { return iq_t<19>(x); }
consteval iq_t<20> operator"" _q20(long double x) { return iq_t<20>(x); }
consteval iq_t<21> operator"" _q21(long double x) { return iq_t<21>(x); }
consteval iq_t<22> operator"" _q22(long double x) { return iq_t<22>(x); }
consteval iq_t<23> operator"" _q23(long double x) { return iq_t<23>(x); }
consteval iq_t<24> operator"" _q24(long double x) { return iq_t<24>(x); }
consteval iq_t<25> operator"" _q25(long double x) { return iq_t<25>(x); }
consteval iq_t<26> operator"" _q26(long double x) { return iq_t<26>(x); }
consteval iq_t<27> operator"" _q27(long double x) { return iq_t<27>(x); }
consteval iq_t<28> operator"" _q28(long double x) { return iq_t<28>(x); }
consteval iq_t<29> operator"" _q29(long double x) { return iq_t<29>(x); }
consteval iq_t<30> operator"" _q30(long double x) { return iq_t<30>(x); }
consteval iq_t<31> operator"" _q31(long double x) { return iq_t<31>(x); }

consteval iq_t<1> operator"" _q1(unsigned long long x) { return iq_t<1>(x); }
consteval iq_t<2> operator"" _q2(unsigned long long x) { return iq_t<2>(x); }
consteval iq_t<3> operator"" _q3(unsigned long long x) { return iq_t<3>(x); }
consteval iq_t<4> operator"" _q4(unsigned long long x) { return iq_t<4>(x); }
consteval iq_t<5> operator"" _q5(unsigned long long x) { return iq_t<5>(x); }
consteval iq_t<6> operator"" _q6(unsigned long long x) { return iq_t<6>(x); }
consteval iq_t<7> operator"" _q7(unsigned long long x) { return iq_t<7>(x); }
consteval iq_t<8> operator"" _q8(unsigned long long x) { return iq_t<8>(x); }
consteval iq_t<9> operator"" _q9(unsigned long long x) { return iq_t<9>(x); }
consteval iq_t<10> operator"" _q10(unsigned long long x) { return iq_t<10>(x); }
consteval iq_t<11> operator"" _q11(unsigned long long x) { return iq_t<11>(x); }
consteval iq_t<12> operator"" _q12(unsigned long long x) { return iq_t<12>(x); }
consteval iq_t<13> operator"" _q13(unsigned long long x) { return iq_t<13>(x); }
consteval iq_t<14> operator"" _q14(unsigned long long x) { return iq_t<14>(x); }
consteval iq_t<15> operator"" _q15(unsigned long long x) { return iq_t<15>(x); }
consteval iq_t<16> operator"" _q16(unsigned long long x) { return iq_t<16>(x); }
consteval iq_t<17> operator"" _q17(unsigned long long x) { return iq_t<17>(x); }
consteval iq_t<18> operator"" _q18(unsigned long long x) { return iq_t<18>(x); }
consteval iq_t<19> operator"" _q19(unsigned long long x) { return iq_t<19>(x); }
consteval iq_t<20> operator"" _q20(unsigned long long x) { return iq_t<20>(x); }
consteval iq_t<21> operator"" _q21(unsigned long long x) { return iq_t<21>(x); }
consteval iq_t<22> operator"" _q22(unsigned long long x) { return iq_t<22>(x); }
consteval iq_t<23> operator"" _q23(unsigned long long x) { return iq_t<23>(x); }
consteval iq_t<24> operator"" _q24(unsigned long long x) { return iq_t<24>(x); }
consteval iq_t<25> operator"" _q25(unsigned long long x) { return iq_t<25>(x); }
consteval iq_t<26> operator"" _q26(unsigned long long x) { return iq_t<26>(x); }
consteval iq_t<27> operator"" _q27(unsigned long long x) { return iq_t<27>(x); }
consteval iq_t<28> operator"" _q28(unsigned long long x) { return iq_t<28>(x); }
consteval iq_t<29> operator"" _q29(unsigned long long x) { return iq_t<29>(x); }
consteval iq_t<30> operator"" _q30(unsigned long long x) { return iq_t<30>(x); }
consteval iq_t<31> operator"" _q31(unsigned long long x) { return iq_t<31>(x); }

using q1 = iq_t<1>;
using q2 = iq_t<2>;
using q3 = iq_t<3>;
using q4 = iq_t<4>;
using q5 = iq_t<5>;
using q6 = iq_t<6>;
using q7 = iq_t<7>;
using q8 = iq_t<8>;
using q9 = iq_t<9>;
using q10 = iq_t<10>;
using q11 = iq_t<11>;
using q12 = iq_t<12>;
using q13 = iq_t<13>;
using q14 = iq_t<14>;
using q15 = iq_t<15>;
using q16 = iq_t<16>;
using q17 = iq_t<17>;
using q18 = iq_t<18>;
using q19 = iq_t<19>;
using q20 = iq_t<20>;
using q21 = iq_t<21>;
using q22 = iq_t<22>;
using q23 = iq_t<23>;
using q24 = iq_t<24>;
using q25 = iq_t<25>;
using q26 = iq_t<26>;
using q27 = iq_t<27>;
using q28 = iq_t<28>;
using q29 = iq_t<29>;
using q30 = iq_t<30>;
using q31 = iq_t<31>;

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> abs(const iq_t<P> iq){
    const auto ivalue = iq.as_i32();
    return iq_t<Q>::from_i32(ivalue > 0 ? ivalue : -ivalue);
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr bool isnormal(const iq_t<P> iq){return iq.as_i32();}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr bool signbit(const iq_t<P> iq){return iq.as_i32() & (1 << 31);}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> sign(const iq_t<P> iq){
    if(likely(iq)) return iq_t<Q>(iq > 0 ? 1 : -1);
    else return iq_t<Q>(0);
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> fmod(const iq_t<P> a, const iq_t<P> b){
    return iq_t<Q>(_iq<P>::from_i32(a.as_i32() % b.as_i32()));}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> lerp(const iq_t<P> x, const iq_t<P> a, const iq_t<P> b){
    return a * (1 - x) + b * x;}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> mean(const iq_t<P> a, const iq_t<P> b){
    return iq_t<Q>(_iq<P>::from_i32((a.as_i32() + b.as_i32()) >> 1));}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> frac(const iq_t<P> iq){
    return iq_t<Q>(_iq<P>::from_i32((iq.as_i32()) & ((1 << P) - 1)));
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> floor(const iq_t<P> iq){
    return int(iq);}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> ceil(const iq_t<P> iq){
    return (iq > int(iq)) ? int(iq) + 1 : int(iq);}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr iq_t<Q> round(const iq_t<P> iq){
    static constexpr auto HALF_ONE = iq_t<Q>(0.5);
    return iq_t<Q>(int(iq + HALF_ONE));}


template<size_t Q>
bool not_in_one(const iq_t<Q> qv){
    // return (qv.as_i32() & (~uint32_t((1u << Q) - 1)));
    if(qv < iq_t<Q>(-0.001)) return true;
    if(qv > iq_t<Q>(1.001)) return true;
    return false;
}

template<size_t Q>
bool is_in_one(const iq_t<Q> qv){
    return not is_in_one(qv);
}

}

namespace std{
    using ymd::iq_t;
    template<size_t Q>
    class numeric_limits<iq_t<Q>> {
    public:
        __fast_inline constexpr static iq_t<Q> infinity() noexcept {return iq_t<Q>(_iq<Q>::from_i32(0x7FFFFFFF));}
        __fast_inline constexpr static iq_t<Q> lowest() noexcept {return iq_t<Q>(_iq<Q>::from_i32(0x80000000));}

        __fast_inline constexpr static iq_t<Q> min() noexcept {return iq_t<Q>(_iq<Q>::from_i32(0x80000000));}
        __fast_inline constexpr static iq_t<Q> max() noexcept {return iq_t<Q>(_iq<Q>::from_i32(0x7FFFFFFF));}
    };
    template<size_t Q>
    struct common_type<iq_t<Q>, float> {
        using type = iq_t<Q>;
    };

    template<size_t Q>
    struct common_type<iq_t<Q>, double> {
        using type = iq_t<Q>;
    };

    template<size_t Q>
    struct common_type<float, iq_t<Q>> {
        using type = iq_t<Q>;
    };

    template<size_t Q>
    struct common_type<double, iq_t<Q>> {
        using type = iq_t<Q>;
    };

    template<size_t Q>
    __fast_inline constexpr auto signbit(const iq_t<Q> iq)  {return ymd::signbit(iq);}
    
    template<size_t Q>
    struct make_signed<iq_t<Q>>{
        using type = iq_t<Q>;
    };

    template<size_t Q>
    struct make_unsigned<iq_t<Q>>{
        using type = iq_t<Q>;
    };
}
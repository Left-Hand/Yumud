#pragma once


#include "sys/core/platform.h"

//部分平台提供了预编译的lib文件 性能可能会更高
#if defined(IQMATH_SPEC_RISCV)
#include "riscv/IQmath_RV32.h"
#elif defined(IQMATH_SPEC_ARM)
#include "arm/IQmath_ARM.h"
#else

#include "_IQNdiv.hpp"
#include "_IQNatan2.hpp"
#include "_IQNtoF.hpp"
#include "_IQFtoN.hpp"
#include "_IQNsqrt.hpp"
#include "_IQNexp.hpp"
#include "_IQNasin_acos.hpp"
#include "_IQNsin_cos.hpp"
#include "_IQNlog.hpp"

#include "rts_support.h"
#include <numeric>

#ifndef LOG_E
#define LOG_E (0.434294481903)
#endif

#endif


//引入浮点的结构模型 以方便进行和浮点数转换
#include "sys/math/float/fp32.hpp"
#include "sys/math/float/fp64.hpp"

template<size_t Q>
requires (Q < 32)
struct iq_t;

namespace std{
    template<size_t Q>
    requires (Q < 32)
    struct is_arithmetic<iq_t<Q>> : std::true_type {};

    template<size_t Q>
    requires (Q < 32)
    struct is_floating_point<iq_t<Q>> : std::false_type {};
}


// 默认模板：非定点数类型
template<typename T>
constexpr bool is_fixed_point_v = false;

// 特化模板：定点数类型
template<size_t Q>
requires (Q < 32)
constexpr bool is_fixed_point_v<iq_t<Q>> = true;

template<size_t Q>
requires (Q < 32)
struct iq_t{
private:
public:
    static constexpr size_t q_num = Q;
    using iq_type = _iq<Q>;

    iq_type value;

    __fast_inline constexpr iq_t():value(_iq<Q>::from_i32(0)){;}

    template<size_t P>
    __fast_inline constexpr iq_t(const _iq<P> otherv) : value(otherv){;}

    __fast_inline explicit constexpr operator _iq<Q>() const {return value;}

    template<size_t P>
    __fast_inline constexpr operator iq_t<P>() const {return iq_t<P>(value);}

    __fast_inline constexpr iq_t(const iq_t<Q> & other):value(other.value){};


    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr iq_t(const T intValue) : value(_iq<Q>::from_i32(intValue << Q)) {;}

    // template<typename T>
    // requires std::is_integral_v<T>
    // struct _CastFromIntHelper{
    // private:
    //     T value_;
    // public:
    //     __fast_inline constexpr _CastFromIntHelper(const T value):
    //         value_(value){;}

    //     __fast_inline constexpr operator T() const{
    //         return value_;
    //     }
    // };

    // template<typename T>
    // requires std::is_integral_v<T>
    // __fast_inline constexpr iq_t(const _CastFromIntHelper<T> helper) : 
    //     value(_iq<Q>::from_i32(T(helper) << Q)) {;}


    #ifdef STRICT_IQ
    __fast_inline consteval explicit iq_t(const float fv):value((std::is_constant_evaluated()) ? _IQFtoN<Q>(fv) : _IQFtoN<Q>(fv)){};
    #else
    __fast_inline constexpr iq_t(const float fv):value((std::is_constant_evaluated()) ? _IQFtoN<Q>(fv) : _IQFtoN<Q>(fv)){};
    #endif

    static __fast_inline constexpr iq_t from (const floating auto fv){return iq_t{_IQFtoN<Q>(fv)};}

    __fast_inline constexpr iq_t operator+() const {
        return *this;
    }

    __fast_inline constexpr iq_t operator-() const {
        return iq_t(_iq<Q>::from_i32(-(value.to_i32())));
    }

    //#region addsub
    template<size_t P>
    __fast_inline constexpr iq_t & operator +=(const iq_t<P> other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(this->value.to_i32() + other.value.to_i32()));
    }

    __fast_inline constexpr iq_t & operator +=(const integral auto other) {
        return *this += iq_t<Q>(other);
    }

    template<size_t P>
    __fast_inline constexpr iq_t & operator -=(const iq_t<P> other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(this->value.to_i32() - other.value.to_i32()));
    }

    __fast_inline constexpr iq_t & operator -=(const integral auto other) {
        return *this -= iq_t<Q>(other);
    }
    //#endregion addsub

    //#region multiply

    template<size_t P>
    __fast_inline constexpr iq_t& operator *=(const iq_t<P> other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(
            (int64_t(this->value.to_i32()) * int64_t((other).value.to_i32())) >> (P)
        ));
    }

    __fast_inline constexpr iq_t& operator *=(const integral auto other) {
        return *this = iq_t<Q>(_iq<Q>::from_i32(this->value.to_i32() * other));
    }
    
    //#endregion

    //#region division
    __fast_inline constexpr iq_t & operator/=(const integral auto other) {
        return *this = iq_t(_iq<Q>::from_i32((value.to_i32() / other)));
    }

    template<size_t P>
    __fast_inline constexpr iq_t & operator/=(const iq_t<P> other) {
        if (std::is_constant_evaluated()) {
            return *this = iq_t(_iq<Q>::from_i32(int32_t((value.to_i32() * int(1 << Q)) / (other.value.to_i32() * int(1 << P))) / int(1 << Q)));
        }else{
            return *this = iq_t(_IQNdiv<Q>(value, _iq<Q>(other.value)));
        }
    }

    //#endregion

    //#region comparisons
    #define IQ_COMP_TEMPLATE(op)\
    template<size_t P>\
    __fast_inline constexpr bool operator op (const iq_t<P> other) const {\
        return value.to_i32() op iq_type(other.value).to_i32();\
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
        return (((value.to_i32()) >> Q) op other);\
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
        return iq_t(_iq<Q>::from_i32(value.to_i32() << shift));
    }

    __fast_inline constexpr iq_t operator>>(int shift) const {
        return iq_t(_iq<Q>::from_i32(value.to_i32() >> shift));
    }
    //#endregion

    __fast_inline constexpr explicit operator bool() const {
        return bool(value.to_i32());
    }

    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr explicit operator T() const {
        return value.to_i32() >> Q;
    }
    

    template<typename T>
    requires std::is_floating_point_v<T>
    __inline constexpr explicit operator T() const{
        if(std::is_constant_evaluated()){
            return float(value.to_i32()) / int(1 << Q);
        }else{
            return _IQNtoF<Q>(value);
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

template<size_t Q>
__fast_inline iq_t<31> sinf(const iq_t<Q> iq){
    return iq_t<31>(_IQNsin<Q>(iq.value));
}

template<size_t Q>
__fast_inline iq_t<31> cosf(const iq_t<Q> iq){
    return iq_t<31>(_IQNcos<Q>(iq.value));
}

template<size_t Q>
__fast_inline iq_t<31> sin(const iq_t<Q> iq){return sinf(iq);}

template<size_t Q>
__fast_inline iq_t<31> cos(const iq_t<Q> iq){return cosf(iq);}

template<size_t Q>
__fast_inline iq_t<Q> tanf(const iq_t<Q> iq) {return iq_t<Q>(sin(iq)) / iq_t<Q>(cos(iq));}

template<size_t Q>
__fast_inline iq_t<Q> tan(const iq_t<Q> iq) {return iq_t<Q>(sin(iq)) / iq_t<Q>(cos(iq));}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<29> asin(const iq_t<Q> iq) {
    {
        return iq_t<29>(_iq<29>(_IQNasin<Q>(iq.value)));
    }
}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<29> acos(const iq_t<Q> iq) {
    {
        return iq_t<29>(_iq<29>::from_i32(
            (int32_t(1.570796327 * (1 << 29)) 
            - std::bit_cast<int32_t>(_IQNasin<Q>(iq.value)))
        ));
    }
}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<29> asinf(const iq_t<Q> iq){return ::asin(iq);}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<29> acosf(const iq_t<Q> iq){return ::acos(iq);}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<Q> atanf(const iq_t<Q> iq) {
    {
        return iq_t<Q>(_iq<Q>(_IQNatan2<Q>(iq.value, _iq<Q>::from_i32(1 << Q))));
    }
}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<Q> atan(const iq_t<Q> iq) {
    return atanf(iq);
}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<Q> atan2f(const iq_t<Q> a, const iq_t<Q> b) {
    {
        return iq_t<Q>(_iq<Q>(_IQNatan2<Q>(a.value,b.value)));
    }
}

template<size_t Q>
requires (Q < 30)
__fast_inline iq_t<Q> atan2(const iq_t<Q> a, const iq_t<Q> b) {
    return atan2f(a, b);
}

template<size_t Q>
__fast_inline iq_t<Q> sqrtf(const iq_t<Q> iq){
    {
        return iq_t<Q>(_iq<Q>(_IQNsqrt<Q>(iq.value)));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> sqrt(const iq_t<Q> iq){
    return sqrtf(iq);
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> abs(const iq_t<Q> iq){
    if(long(iq.value) > 0){
        return iq;
    }else{
        return -iq;
    }
}

template<size_t Q>
__fast_inline constexpr bool isnormal(const iq_t<Q> iq){return bool(iq.value);}
template<size_t Q>
__fast_inline constexpr bool signbit(const iq_t<Q> iq){return std::bit_cast<int32_t>(iq.value) & (1 << 31);}

template<size_t Q>
__fast_inline constexpr iq_t<Q> sign(const iq_t<Q> iq){
    if(likely(long(iq.value))){
        if(long(iq.value) > 0){
            return iq_t<Q>(1);
        }else{
            return iq_t<Q>(-1);
        }
    }else return iq_t<Q>(0);
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> fmod(const iq_t<Q> a, const iq_t<Q> b){return iq_t<Q>(_iq<Q>::from_i32(a.value.to_i32() % b.value.to_i32()));}

template<size_t Q>
__fast_inline constexpr iq_t<Q> lerp(const iq_t<Q> x, const iq_t<Q> a, const iq_t<Q> b){return a * (iq_t<Q>(1) - x) + b * x;}

template<size_t Q>
__fast_inline constexpr iq_t<Q> mean(const iq_t<Q> a, const iq_t<Q> b){return iq_t<Q>(_iq<Q>::from_i32((a.value.to_i32() + b.value.to_i32()) >> 1));}

template<size_t Q>
__fast_inline constexpr iq_t<Q> frac(const iq_t<Q> iq){
    return iq_t<Q>(_iq<Q>::from_i32((iq.value) & ((1 << Q) - 1)));
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> floor(const iq_t<Q> iq){return int(iq);}

template<size_t Q>
__fast_inline constexpr iq_t<Q> ceil(const iq_t<Q> iq){return (iq > int(iq)) ? int(iq) + 1 : int(iq);}

template<size_t Q>
__fast_inline constexpr iq_t<Q> round(const iq_t<Q> iq){return iq_t<Q>((int)_IQint(long(iq.value) + iq_t<Q>::from(0.5)));}


#ifdef IQ_USE_LOG

template<size_t Q>
__fast_inline iq_t<Q> log10(const iq_t<Q> iq) {
    {
        #ifdef IQ_CH32_LOG
        return iq_t(_iq<Q>(_IQlog10(iq.value)));
        #else
        return iq_t(_iq<Q>(_IQNlog<Q>(int32_t(iq.value)))) / iq_t(_iq<Q>(_IQNlog<Q>(int32_t(iq_t<Q>::from(10)))));
        #endif
    }
}

template<size_t Q>
__fast_inline iq_t<Q> log(const iq_t<Q> iq) {
    {
        #ifdef IQ_CH32_LOG
        return iq_t(_iq<Q>(_IQdiv(_IQlog10(iq.value), _IQlog10(_iq<Q>(M_E)))));
        #else
            return iq_t<Q>(_iq<Q>(_IQNlog<Q>(int32_t(iq.value))));
        #endif
    }
}

template<size_t Q>
__fast_inline iq_t<Q> exp(const iq_t<Q> iq) {
    {
        return iq_t<Q>(_iq<Q>(_IQNexp<Q>(iq.value)));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> pow(const iq_t<Q> base, const iq_t<Q> exponent) {
    // if(std::is_constant_evaluated()){
    //     return iq_t(cem::pow(double(base), double(exponent)));
    // }else
    {
        return exp(exponent * log(base));
    }
}

#endif

namespace std{
    template<size_t Q>
    class numeric_limits<iq_t<Q>> {
    public:
        __fast_inline constexpr static iq_t<Q> infinity() noexcept {return iq_t<Q>(_iq<Q>::from_i32(0x80000000));}
        __fast_inline constexpr static iq_t<Q> lowest() noexcept {return iq_t<Q>(_iq<Q>::from_i32(0x7FFFFFFF));}

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
    __fast_inline iq_t<Q> sinf(const iq_t<Q> iq){return ::sinf(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> cosf(const iq_t<Q> iq){return ::cosf(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> sin(const iq_t<Q> iq){return ::sin(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> cos(const iq_t<Q> iq){return ::cos(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> tanf(const iq_t<Q> iq){return ::tanf(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> tan(const iq_t<Q> iq){return ::tan(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> asinf(const iq_t<Q> iq){return ::asin(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> asin(const iq_t<Q> iq){return ::asin(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> acosf(const iq_t<Q> iq){return ::acos(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> acos(const iq_t<Q> iq){return ::acos(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> atan(const iq_t<Q> iq){return ::atan(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> atan2f(const iq_t<Q> a, const iq_t<Q> b){return ::atan2f(a,b);}
    
    template<size_t Q>
    __fast_inline iq_t<Q> atan2(const iq_t<Q> a, const iq_t<Q> b){return ::atan2(a,b);}

    template<size_t Q>
    __fast_inline iq_t<Q> sqrt(const iq_t<Q> iq){return ::sqrt(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> abs(const iq_t<Q> iq){return ::abs(iq);}

    template<size_t Q>
    __fast_inline bool isnormal(const iq_t<Q> iq){return ::isnormal(iq);}

    template<size_t Q>
    __fast_inline bool signbit(const iq_t<Q> iq){return ::signbit(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> fmod(const iq_t<Q> a, const iq_t<Q> b){return ::fmod(a, b);}

    template<size_t Q>
    __fast_inline iq_t<Q> mean(const iq_t<Q> a, const iq_t<Q> b){return ::mean(a, b);}

    template<size_t Q>
    __fast_inline iq_t<Q> frac(const iq_t<Q> iq){return ::frac(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> floor(const iq_t<Q> iq){return ::floor(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> ceil(const iq_t<Q> iq){return ::ceil(iq);}

    #ifdef IQ_USE_LOG

    template<size_t Q>
    __fast_inline iq_t<Q> log10(const iq_t<Q> iq){return ::log10(iq);}

    template<size_t Q>
    __fast_inline iq_t<Q> log(const iq_t<Q> iq){return ::log(iq);}
    #endif
}

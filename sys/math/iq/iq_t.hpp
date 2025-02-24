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

struct _iq{
    int32_t value = 0;
    __fast_inline constexpr explicit _iq(const int32_t _value) : value(_value){;}
    __fast_inline constexpr operator int32_t() const{return value;}
};

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
    __fast_inline scexpr _iq float_to_iq(const float fv){
        if(std::is_constant_evaluated()){
            return _iq{int32_t(fv * int(1 << Q))};
        }

        const int32_t d = std::bit_cast<int32_t>(fv);
        const int32_t exponent = ((d >> 23) & 0xff);
        const uint64_t mantissa = (exponent == 0) ? (0) : ((d & ((1 << 23) - 1)) | (1 << 23));

        const uint64_t temp = [&] -> uint64_t {
            if(exponent == 0 or exponent == 0xff){
                return 0;
            }else{
                return LSHIFT(mantissa, exponent - 127);
            }
        }();

        const uint64_t uresult = RSHIFT(temp, (23 - Q));
        const int32_t result = d > 0 ? uresult : -uresult;

        if((bool(d > 0) ^ bool(result > 0)) or (uresult > (uint64_t)0x80000000)){//OVERFLOW
            if(d > 0){
                return std::bit_cast<_iq>(0x7FFFFFFF);
            }else{
                return std::bit_cast<_iq>(0x80000000);
            }
        }else{
            return std::bit_cast<_iq>(result);
        }

    }
public:
    static constexpr size_t q_num = Q;
    _iq value;

    __fast_inline constexpr iq_t():value(0){;}

    __fast_inline constexpr iq_t(const _iq & other) : value(other){;}
    __fast_inline explicit constexpr operator _iq() const {return value;}

    template<size_t P>
    __fast_inline constexpr operator iq_t<P>() const {return iq_t<P>(_iq((int64_t(value) << Q) >> P));}

    __fast_inline constexpr iq_t(const iq_t<Q> & other):value(other.value){};


    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr iq_t(const T intValue) : value(intValue << Q) {;}

    #ifdef STRICT_IQ
    __fast_inline consteval explicit iq_t(const float fv):value((std::is_constant_evaluated()) ? float_to_iq(fv) : float_to_iq(fv)){};
    #else
    __fast_inline constexpr iq_t(const float fv):value((std::is_constant_evaluated()) ? float_to_iq(fv) : float_to_iq(fv)){};
    #endif

    static __fast_inline constexpr iq_t from (const floating auto fv){return iq_t{float_to_iq(fv)};}


    __fast_inline constexpr iq_t operator+() const {
        return iq_t(_iq(value));
    }
    
    __fast_inline constexpr iq_t operator-() const {
        return iq_t(_iq(-int32_t(value)));
    }

    //#region addsub
    #define IQ_ADD_SUB_TEMPLATE(op)\
    template<size_t P>\
    __fast_inline constexpr iq_t operator op(const iq_t<P> other) const {\
        if constexpr (Q == P){\
            return iq_t(_iq(int32_t(value) op int32_t(other.value)));\
        }else{\
            return iq_t(_iq(int32_t(value) op ((int64_t(other.value) << Q) >> P )));\
        }\
    }\
    __fast_inline constexpr iq_t & operator op##=(iq_t other) {\
        return *this = *this op other;\
    }\
    template<integral T>\
    __fast_inline constexpr iq_t operator op (T other) const {\
        return *this op iq_t(other);\
    }\
    template<integral T>\
    __fast_inline constexpr iq_t & operator op##=(T other) {\
        return *this = *this op iq_t(other);\
    }\

    IQ_ADD_SUB_TEMPLATE(+)
    IQ_ADD_SUB_TEMPLATE(-)
    #undef IQ_ADD_SUB_TEMPLATE
    //#endregion addsub

    //#region multiply
    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr iq_t operator*(const T other) const {
        return iq_t(_iq(int32_t(value) * other));
    }

    template<size_t P>
    __fast_inline constexpr iq_t operator*(const iq_t<P> other) const {
        return iq_t((_iq)(((int64_t)value * (int64_t)other.value) >> P));
    }

    template<size_t P>
    __fast_inline constexpr iq_t& operator*=(const iq_t<P> other) {
        *this = *this * other;
        return *this;
    }
    
    //#endregion

    //#region division
    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr iq_t operator/(const T other) const {
        return iq_t(_iq((int32_t(value) / other)));
    }

    template<size_t P>
    __fast_inline constexpr iq_t operator/(const iq_t<P> other) const {
        if (std::is_constant_evaluated()) {
            return iq_t((_iq)((int32_t)value / (int32_t)other.value << Q));
        }else{
            return iq_t(_iq(_IQNdiv<Q>(int32_t(value), ((int64_t(other.value) << Q) >> P))));
        }
    }

    template<size_t P>
    __fast_inline constexpr iq_t& operator/=(const iq_t<P> other) {
        *this = *this / other;
        return *this;
    }
    //#endregion

    //#region comparisons
    #define IQ_COMP_TEMPLATE(op)\
    template<size_t P>\
    __fast_inline constexpr bool operator op (const iq_t<P> other) const {\
        if constexpr (Q == P){\
            return (int32_t(value) op int32_t(other.value));\
        }else{\
            return (int32_t(value) op ((int64_t(other.value) << Q) >> P));\
        }\
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
        return ((int32_t(this->value) >> Q) op other);\
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
        return iq_t(_iq(int32_t(value) << shift));
    }

    __fast_inline constexpr iq_t operator>>(int shift) const {
        return iq_t(_iq(int32_t(value) >> shift));
    }
    //#endregion

    __fast_inline constexpr operator bool() const {
        return bool(int32_t(value));
    }

    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr explicit operator T() const {
        return int32_t(value) >> Q;
    }
    

    template<typename T>
    requires std::is_floating_point_v<T>
    __inline constexpr explicit operator T() const{
        if(std::is_constant_evaluated()){
            return float(int32_t(value)) / int(1 << Q);
        }else{
            return _IQNtoF<Q>(int32_t(value));
        }
    }
};


template<size_t Q>
__fast_inline constexpr iq_t<Q> operator +(const integral auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) + iq_v;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator -(const integral auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) - iq_v;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator *(const integral auto val, const iq_t<Q> iq_v) {
	return iq_v * val;
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> operator /(const integral auto val, const iq_t<Q> iq_v) {
	return iq_t<Q>(val) / iq_v;
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
__fast_inline iq_t<Q> sinf(const iq_t<Q> iq){
    {
        return iq_t<Q>(_iq(_IQNsin<Q>(iq.value)));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> cosf(const iq_t<Q> iq){
    {
        return iq_t<Q>(_iq(_IQNcos<Q>(iq.value)));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> sin(const iq_t<Q> iq){return sinf(iq);}

template<size_t Q>
__fast_inline iq_t<Q> cos(const iq_t<Q> iq){return cosf(iq);}

template<size_t Q>
__fast_inline iq_t<Q> tanf(const iq_t<Q> iq) {return sin(iq) / cos(iq);}

template<size_t Q>
__fast_inline iq_t<Q> tan(const iq_t<Q> iq) {return tanf(iq);}

template<size_t Q>
__fast_inline iq_t<Q> asin(const iq_t<Q> iq) {
    {
        return iq_t<Q>(_iq(_IQNasin<Q>(iq.value)));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> acos(const iq_t<Q> iq) {
    {
        return iq_t<Q>(_iq((int32_t(1.570796327 * (1 << Q)) - _IQNasin<Q>(int32_t(iq.value)))));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> asinf(const iq_t<Q> iq){return ::asin(iq);}

template<size_t Q>
__fast_inline iq_t<Q> acosf(const iq_t<Q> iq){return ::acos(iq);}

template<size_t Q>
__fast_inline iq_t<Q> atan(const iq_t<Q> iq) {
    {
        return iq_t<Q>(_iq(_IQNatan2<Q>(iq.value, int(1 << Q))));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> atan2f(const iq_t<Q> a, const iq_t<Q> b) {
    {
        return iq_t<Q>(_iq(_IQNatan2<Q>(a.value,b.value)));
    }
}

template<size_t Q>
__fast_inline iq_t<Q> atan2(const iq_t<Q> a, const iq_t<Q> b) {
    return atan2f(a, b);
}

template<size_t Q>
__fast_inline iq_t<Q> sqrt(const iq_t<Q> iq){
    {
        return iq_t<Q>(_iq(_IQNsqrt<Q>(iq.value)));
    }
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
    if(long(iq.value)){
        if(long(iq.value) > 0){
            return iq_t<Q>(1);
        }else{
            return iq_t<Q>(-1);
        }
    }else return iq_t<Q>(0);
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> fmod(const iq_t<Q> a, const iq_t<Q> b){return iq_t<Q>(_iq(a.value % b.value));}

template<size_t Q>
__fast_inline constexpr iq_t<Q> lerp(const iq_t<Q> x, const iq_t<Q> a, const iq_t<Q> b){return a * (iq_t<Q>(1) - x) + b * x;}

template<size_t Q>
__fast_inline constexpr iq_t<Q> mean(const iq_t<Q> a, const iq_t<Q> b){return iq_t<Q>(_iq((a.value + b.value) >> 1));}

template<size_t Q>
__fast_inline constexpr iq_t<Q> frac(const iq_t<Q> iq){
    return iq_t<Q>(_iq((iq.value) & ((1 << Q) - 1)));
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
        return iq_t(_iq(_IQlog10(iq.value)));
        #else
        return iq_t(_iq(_IQNlog<Q>(int32_t(iq.value)))) / iq_t(_iq(_IQNlog<Q>(int32_t(iq_t<Q>::from(10)))));
        #endif
    }
}

template<size_t Q>
__fast_inline iq_t<Q> log(const iq_t<Q> iq) {
    {
        #ifdef IQ_CH32_LOG
        return iq_t(_iq(_IQdiv(_IQlog10(iq.value), _IQlog10(_IQ(M_E)))));
        #else
            return iq_t<Q>(_iq(_IQNlog<Q>(int32_t(iq.value))));
        #endif
    }
}

template<size_t Q>
__fast_inline iq_t<Q> exp(const iq_t<Q> iq) {
    {
        return iq_t<Q>(_iq(_IQNexp<Q>(int32_t(iq.value))));
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

// template<size_t Q>
// __fast_inline iq_t<Q> powfi(const iq_t<Q> base, const int exponent) {
//     if(0 == exponent) {
//         return iq_t<Q>(1);
//     }else if(1 == exponent){
//         return base;
//     }else{
//         iq_t<Q> ret;
//         if(1 < exponent){
//             for(size_t i = 1; i < size_t(exponent); ++i){
//                 ret *= base;
//             }
//         }else{
//             for(size_t i = 1; i < size_t(-exponent); ++i){
//                 ret /= base;
//             }
//         }
//         return ret;
//     }
// }

// template<size_t Q>
// __fast_inline iq_t<Q> powi(const iq_t<Q> base, const int exponent) {
//     return powfi(base, exponent);
// }


namespace std{
    template<size_t Q>
    class numeric_limits<iq_t<Q>> {
    public:
        __fast_inline constexpr static iq_t<Q> infinity() noexcept {return iq_t<Q>(_iq(0x80000000));}
        __fast_inline constexpr static iq_t<Q> lowest() noexcept {return iq_t<Q>(_iq(0x7FFFFFFF));}

        __fast_inline constexpr static iq_t<Q> min() noexcept {return iq_t<Q>(_iq(0x80000000));}
        __fast_inline constexpr static iq_t<Q> max() noexcept {return iq_t<Q>(_iq(0x7FFFFFFF));}
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

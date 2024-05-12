#ifndef __IQT_HPP__

#define __IQT_HPP__


#include "stdint.h"
#include "src/platform.h"
#include "extra_convs.hpp"

#include <IQmath_RV32.h>
#include <type_traits>
#include <limits>


struct iq_t{
public:
    _iq value = 0;

    __fast_inline iq_t(): value(0){;}
    __fast_inline_constexpr explicit iq_t(const _iq & iqValue): value(iqValue){;}

    __fast_inline_constexpr iq_t(const int & intValue) : value(_IQ(intValue)) {;}

    __fast_inline_constexpr iq_t(const int8_t & intValue) : value(_IQ(intValue)) {;}
    __fast_inline_constexpr iq_t(const int16_t & intValue) : value(_IQ(intValue)) {;}
    // int32_t could cause ambigous with _iq(aka long) but int won`t

    __fast_inline_constexpr iq_t(const uint8_t & intValue) : value(_IQ(intValue)) {;}
    __fast_inline_constexpr iq_t(const uint16_t & intValue) : value(_IQ(intValue)) {;}
    __fast_inline_constexpr iq_t(const uint32_t & intValue) : value(_IQ(intValue)) {;}

    __fast_inline_constexpr iq_t(const float & floatValue) : value(_IQ(floatValue)) {;}
    __fast_inline_constexpr iq_t(const double & doubleValue) : value(_IQ(doubleValue)) {;}
    
    explicit iq_t(const String & str);


    __fast_inline_constexpr iq_t operator+(const iq_t & other) const {
        return iq_t(value + other.value);
    }

    __fast_inline_constexpr iq_t operator-(const iq_t & other) const {
        return iq_t(value - other.value);
    }

    __fast_inline_constexpr iq_t operator-() const {
        return iq_t(-value);
    }

    __fast_inline_constexpr iq_t operator*(const iq_t & other) const {
    if (std::is_constant_evaluated()) {
        return iq_t((_iq)((int64_t)value * (int64_t)other.value >> GLOBAL_Q));
    }
    return iq_t(_IQmpy(value, other.value));
    }

    __fast_inline_constexpr iq_t operator/(const iq_t & other) const {
    if (std::is_constant_evaluated()) {
        return iq_t((_iq)((int64_t)value / (int64_t)other.value << GLOBAL_Q));
    }
        return iq_t(_IQdiv(value, other.value));
    }

    __fast_inline_constexpr iq_t& operator+=(const iq_t& other) {
        value = value + other.value;
        return *this;
    }

    __fast_inline_constexpr iq_t& operator-=(const iq_t& other) {
        value = value - other.value;
        return *this;
    }

    __fast_inline_constexpr iq_t& operator*=(const iq_t& other) {
        *this = *this * other;
        return *this;
    }

    __fast_inline_constexpr iq_t& operator/=(const iq_t& other) {
        *this = *this / other;
        return *this;
    }

    __fast_inline_constexpr bool operator==(const auto & other) const {
        return value == static_cast<iq_t>(other).value;
    }

    __fast_inline_constexpr bool operator!=(const auto & other) const {
        return value != static_cast<iq_t>(other).value;
    }

    __fast_inline_constexpr bool operator>(const auto & other) const {
        return value > static_cast<iq_t>(other).value;
    }

    __fast_inline_constexpr bool operator<(const auto & other) const {
        return value < static_cast<iq_t>(other).value;
    }

    __fast_inline_constexpr bool operator>=(const auto & other) const {
        return value >= static_cast<iq_t>(other).value;
    }

    __fast_inline_constexpr bool operator<=(const auto & other) const {
        return value <= static_cast<iq_t>(other).value;
    }

    #ifndef STRICT_IQ

    template <typename U>
    __fast_inline iq_t operator+(const U & other) const {
        return iq_t(value + iq_t(other).value);
    }

    template <typename U>
    __fast_inline iq_t operator-(const U & other) const {
        return iq_t(value - iq_t(other).value);
    }


    __fast_inline iq_t operator*(const int & other) const {
        return iq_t(value * other);
    }

    template <typename U>
    __fast_inline iq_t operator*(const U & other) const {
        return *this * iq_t(other);
    }

    __fast_inline iq_t operator/(const int & other) const {
        return iq_t((value / other));
    }

    __fast_inline iq_t operator/(const float & other) const {
        return *this / iq_t(other);
    }

    __fast_inline iq_t operator/(const double & other) const {
        return *this / iq_t(other);
    }

    __fast_inline iq_t& operator+=(const float & other) {
        *this += iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator+=(const double & other) {
        *this += iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator+=(const int & other) {
        *this += iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator-=(const float & other) {
        *this -= iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator-=(const double & other) {
        *this -= iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator-=(const int & other) {
        *this -= iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator*=(const int & other) {
        value *= other;
        return *this;
    }

    __fast_inline iq_t& operator*=(const float & other) {
        *this *= iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator*=(const double & other) {
        *this *= iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator/=(const int & other) {
        value /= other;
        return *this;
    }

    __fast_inline iq_t& operator/=(const float & other) {
        *this /= iq_t(other);
        return *this;
    }

    __fast_inline iq_t& operator/=(const double & other) {
        *this /= iq_t(other);
        return *this;
    }

    #define IQ_OPERATOR_TEMPLATE(op)\
    __fast_inline bool operator op (const int & other) const {\
        return *this op iq_t(other);\
    }

    IQ_OPERATOR_TEMPLATE(==);
    IQ_OPERATOR_TEMPLATE(!=);
    IQ_OPERATOR_TEMPLATE(>);
    IQ_OPERATOR_TEMPLATE(<);
    IQ_OPERATOR_TEMPLATE(>=);
    IQ_OPERATOR_TEMPLATE(<=);

    #undef IQ_OPERATOR_TEMPLATE

    __fast_inline iq_t& operator=(const int & other){
        value = _IQ(other);
        return *this;
    }

    __fast_inline iq_t& operator=(const float & other){
        value = _IQ(other);
        return *this;
    }

    __fast_inline iq_t& operator=(const double & other){
        value = _IQ(other);
        return *this;
    }

    #endif


    __fast_inline explicit operator bool() const {
        return bool(value);
    }

    #define IQ_INT_TEMPLATE(op)\
    __fast_inline explicit operator op() const {\
        return op(_IQint(value));\
    }

    IQ_INT_TEMPLATE(int);
    IQ_INT_TEMPLATE(int8_t);
    IQ_INT_TEMPLATE(int16_t);
    IQ_INT_TEMPLATE(int32_t);
    IQ_INT_TEMPLATE(uint8_t);
    IQ_INT_TEMPLATE(uint16_t);
    IQ_INT_TEMPLATE(uint32_t);

    #undef IQ_INT_TEMPLATE

    __fast_inline explicit operator float() const{
        return _IQtoF(value);
    }

    __fast_inline explicit operator double() const{
        return _IQtoD(value);
    }

    __no_inline explicit operator String() const;
    String toString(const uint8_t eps = 3) const;
};

#ifndef STRICT_IQ

#define IQ_OP_TEMPLATE(type, op)\
__fast_inline iq_t operator op (type val, const iq_t & iq_v) {\
	return iq_v op iq_t(val);\
}\

IQ_OP_TEMPLATE(int, +);
IQ_OP_TEMPLATE(float, +);
IQ_OP_TEMPLATE(double, +);

IQ_OP_TEMPLATE(int, -);
IQ_OP_TEMPLATE(float, -);
IQ_OP_TEMPLATE(double, -);

IQ_OP_TEMPLATE(int, *);
IQ_OP_TEMPLATE(float, *);

IQ_OP_TEMPLATE(double, *);
IQ_OP_TEMPLATE(int, /);
IQ_OP_TEMPLATE(float, /);
IQ_OP_TEMPLATE(double, /);

#undef IQ_BINA_OP_TEMPLATE


#endif

__fast_inline iq_t sin(const iq_t & iq){return iq_t(_IQsin(iq.value));}

__fast_inline iq_t cos(const iq_t & iq){return iq_t(_IQcos(iq.value));}

__fast_inline iq_t sinf(const iq_t & iq){return iq_t(_IQsin(iq.value));}

__fast_inline iq_t cosf(const iq_t & iq){return iq_t(_IQcos(iq.value));}

__fast_inline iq_t tan(const iq_t & iq) {return iq_t(_IQsin(iq.value) / _IQcos(iq.value));}

__fast_inline iq_t asin(const iq_t & iq) {return iq_t(_IQasin(iq.value));}

__fast_inline iq_t acos(const iq_t & iq) {return iq_t(_IQacos(iq.value));}

__fast_inline iq_t atan(const iq_t & iq) {return iq_t(_IQatan(iq.value));}

__fast_inline iq_t atan2(const iq_t & a, const iq_t & b) {return iq_t(_IQatan2(a.value,b.value));}

__fast_inline iq_t sqrt(const iq_t & iq){return iq_t(_IQsqrt(iq.value));}

__fast_inline iq_t abs(const iq_t & iq) {return iq_t(iq.value > 0 ? iq.value : -iq.value);}

__fast_inline bool isnormal(const iq_t & iq){return bool(iq.value);}

__fast_inline bool signbit(const iq_t & iq){return bool(iq.value < 0);}

__fast_inline iq_t sign(const iq_t & iq);

__fast_inline iq_t fmod(const iq_t & a, const iq_t & b){
    // return iq_t((a.value < b.value && (0 <= a.value)) ? a.value : _IQmpy(_IQfrac(_IQdiv(a.value, b.value)), b.value));
    return iq_t(_iq(a.value % b.value));
}


__fast_inline iq_t lerp(const iq_t & x, const iq_t & a, const iq_t & b){return a * (iq_t(1) - x) + b * x;}
__fast_inline iq_t mean(const iq_t & a, const iq_t & b){return iq_t((a.value + b.value) >> 1);}

__fast_inline iq_t frac(const iq_t & iq){return iq_t(_IQfrac(iq.value));}

__fast_inline iq_t floor(const iq_t & iq){return iq_t(iq.value - _IQfrac(iq.value));}

__fast_inline iq_t round(const iq_t & iq){return iq_t((int)_IQint(iq.value + _IQ(0.5)));}

bool is_equal_approx(const iq_t & a,const iq_t & b);

bool is_equal_approx_ratio(const iq_t a, const iq_t & b, iq_t epsilon = iq_t(CMP_EPSILON), iq_t min_epsilon = iq_t(CMP_EPSILON));

#ifdef IQ_USE_LOG

__fast_inline iq_t log10(const iq_t & iq) {return iq_t(_IQlog10(iq.value));}

__fast_inline iq_t log(const iq_t & iq) {return iq_t(_IQdiv(_IQlog10(iq.value), _IQlog10(_IQ(LOGE))));}

__fast_inline iq_t exp(const iq_t & iq) {return iq_t(_IQexp(iq.value));}

__fast_inline iq_t exp2(const iq_t & iq) {return iq_t(_IQexp2(iq.value));}

__fast_inline iq_t pow(const iq_t & base, const iq_t & exponent) {return iq_t(_IQexp(_IQmpy(exponent.value, _IQdiv(base.value, _IQlog10(_IQ(LOGE))))));}
#endif


__fast_inline void u16_to_uni(const uint16_t & data, iq_t & qv){
#if GLOBAL_Q >= 16
    qv.value = data << (GLOBAL_Q - 16);
#else
    qv.value = data >> (16 - GLOBAL_Q);
#endif
}

__fast_inline constexpr void s16_to_uni(const int16_t & data, iq_t & qv){
    qv.value = data > 0 ? data : -((_iq)-data);
}

__fast_inline void uni_to_u16(const iq_t & qv, uint16_t & data){
#if GLOBAL_Q >= 16
    data = qv.value >> (GLOBAL_Q - 16);
#else
    data = qv.value << (16 - GLOBAL_Q);
#endif
    if(data == 0 && qv.value != 0) data = 0xffff;
}

__fast_inline iq_t sign(const iq_t & iq){
    if(iq.value){
        if(iq.value > 0){
            return iq_t(1);
        }else{
            return iq_t(-1);
        }
    }else return iq_t(0);
}

namespace std{
    template<>
    class numeric_limits<iq_t> {
    public:
        __fast_inline static iq_t infinity() noexcept {return iq_t(std::numeric_limits<float>::infinity());}
        __fast_inline static iq_t lowest() noexcept {return iq_t(-std::numeric_limits<float>::infinity());}
    };
}

#endif
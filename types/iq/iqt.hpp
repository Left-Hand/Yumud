#ifndef __IQT_HPP__

#define __IQT_HPP__


#include "stdint.h"
#include "sys/platform.h"
#include "extra_convs.hpp"
#include "IQmath_RV32.h"

#include <type_traits>
#include <limits>

#include "dsp/constexprmath/ConstexprMath.hpp"

struct _iq{
    _iq16 value = 0;
    __fast_inline_constexpr _iq(const _iq16 & _value) : value(_value){;}
    __fast_inline_constexpr operator _iq16() const{return value;}
};

struct iq_t{
public:
    _iq value = 0;

    __fast_inline iq_t():value(0){;}
    __fast_inline_constexpr explicit iq_t(const _iq & iqValue): value(iqValue.value){;}

    __fast_inline_constexpr iq_t(const int & intValue) : value(_IQ(intValue)) {;}

    __fast_inline_constexpr iq_t(const int8_t & intValue) : value(_IQ(intValue)) {;}
    __fast_inline_constexpr iq_t(const int16_t & intValue) : value(_IQ(intValue)) {;}
    __fast_inline_constexpr iq_t(const int32_t & intValue) : value(_IQ(intValue)) {;}

    __fast_inline_constexpr iq_t(const uint8_t & intValue) : value(_IQ(intValue)) {;}
    __fast_inline_constexpr iq_t(const uint16_t & intValue) : value(_IQ(intValue)) {;}
    __fast_inline_constexpr iq_t(const uint32_t & intValue) : value(_IQ(intValue)) {;}

    __fast_inline_constexpr iq_t(const float & floatValue) : value(_IQ(floatValue)) {;}
    __fast_inline_constexpr iq_t(const double & doubleValue) : value(_IQ(doubleValue)) {;}
    
    explicit iq_t(const String & str);

    __fast_inline_constexpr iq_t operator+(const iq_t & other) const {
        return iq_t(_iq(value + other.value));
    }

    __fast_inline_constexpr iq_t operator-(const iq_t & other) const {
        return iq_t(_iq(value - other.value));
    }

    __fast_inline_constexpr iq_t operator-() const {
        return iq_t(_iq(-value));
    }

    __fast_inline_constexpr iq_t operator*(const iq_t & other) const {
        if (std::is_constant_evaluated()) {
            return iq_t((_iq)((int64_t)value * (int64_t)other.value >> GLOBAL_Q));
        }else{
            return iq_t(_iq(_IQmpy(value, other.value)));
        }
    }


    __fast_inline_constexpr iq_t operator/(const iq_t & other) const {
        if (std::is_constant_evaluated()) {
            return iq_t((_iq)((int64_t)value / (int64_t)other.value << GLOBAL_Q));
        }else{
            return iq_t(_iq(_IQdiv(value, other.value)));
        }
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

    __fast_inline_constexpr iq_t operator*(const int & other) const {
        return iq_t(_iq(value * other));
    }

    __fast_inline_constexpr iq_t operator*(const auto & other) const {
        return *this * iq_t(other);
    }

    __fast_inline_constexpr iq_t operator/(const int & other) const {
        return iq_t(_iq((value / other)));
    }

    __fast_inline_constexpr iq_t operator/(const auto & other) const {
        return *this * iq_t(other);
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

    __fast_inline_constexpr iq_t operator+(const auto & other) const {
        return iq_t(_iq(value + iq_t(other).value));
    }

    __fast_inline_constexpr iq_t operator-(const auto & other) const {
        return iq_t(_iq(value - iq_t(other).value));
    }

    __fast_inline_constexpr iq_t operator<<(int shift) const {
        return iq_t(_iq(value << shift));
    }

    __fast_inline_constexpr iq_t operator>>(int shift) const {
        return iq_t(_iq(value >> shift));
    }

    __fast_inline_constexpr iq_t& operator+=(const auto & other) {
        *this += iq_t(other);
        return *this;
    }

    __fast_inline_constexpr iq_t& operator-=(const auto & other) {
        *this -= iq_t(other);
        return *this;
    }


    #define IQ_OPERATOR_TEMPLATE(op)\
    __fast_inline_constexpr bool operator op (const int & other) const {\
        return *this op iq_t(other);\
    }

    IQ_OPERATOR_TEMPLATE(==);
    IQ_OPERATOR_TEMPLATE(!=);
    IQ_OPERATOR_TEMPLATE(>);
    IQ_OPERATOR_TEMPLATE(<);
    IQ_OPERATOR_TEMPLATE(>=);
    IQ_OPERATOR_TEMPLATE(<=);

    #undef IQ_OPERATOR_TEMPLATE

    __fast_inline_constexpr iq_t& operator=(const auto & other){
        *this = iq_t(other);
        return *this;
    }

    __fast_inline_constexpr explicit operator bool() const {
        return bool(value);
    }

    #define IQ_INT_TEMPLATE(op)\
    __fast_inline_constexpr explicit operator op() const {\
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

    __fast_inline_constexpr explicit operator float() const{
        if(std::is_constant_evaluated()){
            return float(this->value) / (1 << GLOBAL_Q);
        }else{
            return _IQtoF(value);
        }
    }

    __fast_inline_constexpr explicit operator double() const{
        if(std::is_constant_evaluated()){
            return double(this->value) / (1 << GLOBAL_Q);
        }else{
            return _IQtoD(value);
        }
    }

    __no_inline explicit operator String() const;
    String toString(unsigned char eps = 3) const;
};

#ifndef STRICT_IQ

#define IQ_OP_TEMPLATE(type, op)\
__fast_inline_constexpr iq_t operator op (type val, const iq_t & iq_v) {\
	return iq_t(val) op iq_v;\
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

using cem = ConstexprMath;

__fast_inline_constexpr iq_t sinf(const iq_t & iq){
    if (std::is_constant_evaluated()) {
        return cem::sin(double(iq));
    }
    return iq_t(_iq(_IQsin(iq.value)));
}

__fast_inline_constexpr iq_t cosf(const iq_t & iq){
    if (std::is_constant_evaluated()) {
        return cem::cos(double(iq));
    }
    return iq_t(_iq(_IQcos(iq.value)));
}

__fast_inline_constexpr iq_t sin(const iq_t & iq){return sinf(iq);}

__fast_inline_constexpr iq_t cos(const iq_t & iq){return cosf(iq);}

__fast_inline_constexpr iq_t tan(const iq_t & iq) {return sin(iq) / cos(iq);}

__fast_inline_constexpr iq_t asin(const iq_t & iq) {
    if (std::is_constant_evaluated()) {
        return cem::asin(double(iq));
    }else{
        return iq_t(_iq(_IQasin(iq.value)));
    }
}

__fast_inline_constexpr iq_t acos(const iq_t & iq) {
    if (std::is_constant_evaluated()) {
        return cem::acos(double(iq));
    }else{
        return iq_t(_iq(_IQacos(iq.value)));
    }
}

__fast_inline_constexpr iq_t atan(const iq_t & iq) {
    if (std::is_constant_evaluated()) {
        return cem::atan(double(iq));
    }else{
        return iq_t(_iq(_IQatan(iq.value)));
    }
}

__fast_inline_constexpr iq_t atan2(const iq_t & a, const iq_t & b) {
    if (std::is_constant_evaluated()) {
        return cem::atan(atan2(a, b));
    }else{
        return iq_t(_iq(_IQatan2(a.value,b.value)));
    }
}

__fast_inline_constexpr iq_t sqrt(const iq_t & iq){
    if(std::is_constant_evaluated()) {
        return cem::sqrt(double(iq));
    }else{
        return iq_t(_iq(_IQsqrt(iq.value)));
    }
}

__fast_inline_constexpr iq_t abs(const iq_t & iq) {return iq.value > 0 ? iq : -iq;}

__fast_inline_constexpr bool isnormal(const iq_t & iq){return bool(iq.value);}

__fast_inline_constexpr bool signbit(const iq_t & iq){return bool(iq.value < 0);}


__fast_inline_constexpr iq_t sign(const iq_t & iq){
    if(iq.value){
        if(iq.value > 0){
            return iq_t(1);
        }else{
            return iq_t(-1);
        }
    }else return iq_t(0);
}

__fast_inline_constexpr iq_t fmod(const iq_t & a, const iq_t & b){return iq_t(_iq(a.value % b.value));}


__fast_inline_constexpr iq_t lerp(const iq_t & x, const iq_t & a, const iq_t & b){return a * (iq_t(1) - x) + b * x;}
__fast_inline_constexpr iq_t mean(const iq_t & a, const iq_t & b){return iq_t(_iq((a.value + b.value) >> 1));}

__fast_inline_constexpr iq_t frac(const iq_t & iq){
    if(std::is_constant_evaluated()) {
            return iq - int(iq);
    }
    else return iq_t(_iq(_IQfrac(iq.value)));
}

__fast_inline_constexpr iq_t floor(const iq_t & iq){return int(iq);}

__fast_inline_constexpr iq_t round(const iq_t & iq){return iq_t((int)_IQint(iq.value + _IQ(0.5)));}

bool is_equal_approx(const iq_t & a,const iq_t & b);

bool is_equal_approx_ratio(const iq_t a, const iq_t & b, iq_t epsilon = iq_t(CMP_EPSILON), iq_t min_epsilon = iq_t(CMP_EPSILON));

#define IQ_USE_LOG
#ifdef IQ_USE_LOG

__fast_inline_constexpr iq_t log10(const iq_t & iq) {
    if(std::is_constant_evaluated()){
        return cem::ln(double(iq)) / cem::ln(10.0);
    }else{
        return iq_t(_iq(_IQlog10(iq.value)));
    }
}

__fast_inline_constexpr iq_t log(const iq_t & iq) {
    if(std::is_constant_evaluated()){
        return cem::ln(double(iq));
    }else{
        return iq_t(_iq(_IQdiv(_IQlog10(iq.value), _IQlog10(_IQ(M_E)))));
    }
}

__fast_inline_constexpr iq_t exp(const iq_t & iq) {
    if(std::is_constant_evaluated()){
        return cem::exp(double(iq));
    }else{
        return iq_t(_iq(_IQexp(iq.value)));
    }
}

__fast_inline_constexpr iq_t exp2(const iq_t & iq) {
    if(std::is_constant_evaluated()){
        return cem::pow(2.0, double(iq));
    }else{
        return iq_t(_iq(_IQexp2(iq.value)));
    }
}

__fast_inline_constexpr iq_t pow(const iq_t & base, const iq_t & exponent) {
    if(std::is_constant_evaluated()){
        return cem::pow(double(base), double(exponent));
    }else{
        return iq_t(_iq(_IQexp(_IQmpy(exponent.value, _IQdiv(base.value, _IQlog10(_IQ(LOGE)))))));
    }
}
#endif

__fast_inline_constexpr void u16_to_uni(const uint16_t & data, iq_t & qv){
#if GLOBAL_Q > 16
    qv.value = data << (GLOBAL_Q - 16);
#elif(GLOBAL_Q < 16)
    qv.value = data >> (16 - GLOBAL_Q);
#else
    qv.value = data;
#endif

}

__fast_inline_constexpr void s16_to_uni(const int16_t data, iq_t & qv){
    qv.value = data > 0 ? data : -((_iq)-data);
}

__fast_inline_constexpr void uni_to_u16(const iq_t qv, uint16_t & data){
#if GLOBAL_Q >= 16
    data = qv.value >> (GLOBAL_Q - 16);
#else
    data = qv.value << (16 - GLOBAL_Q);
#endif
    if(data == 0 && qv.value != 0) data = 0xffff;
}


namespace std{
    template<>
    class numeric_limits<iq_t> {
    public:
        __fast_inline static iq_t infinity() noexcept {return iq_t((1 << GLOBAL_Q) - 1);}
        __fast_inline static iq_t lowest() noexcept {return iq_t(-(1 << GLOBAL_Q));}
    };

    typedef std::common_type<iq_t, float>::type real_t;
    typedef std::common_type<iq_t, double>::type real_t;
    typedef std::common_type<iq_t, int>::type real_t;

    typedef std::common_type<float, iq_t>::type real_t;
    typedef std::common_type<double, iq_t>::type real_t;
    typedef std::common_type<int, iq_t>::type real_t;
}

#endif
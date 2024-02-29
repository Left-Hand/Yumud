#ifndef __IQT_HPP__

#define __IQT_HPP__


#include "stdint.h"
#include "src/comm_inc.h"
#include "types/string/String.hpp"

#include <IQmath_RV32.h>
#include <limits>


struct iq_t{
public:
    volatile _iq value = 0;

    __fast_inline iq_t(): value(0){;}
    __fast_inline explicit iq_t(const _iq & iqValue): value(iqValue){;}
    __fast_inline explicit iq_t(const int & intValue) : value(_IQ(intValue)) {;}
    __fast_inline explicit iq_t(const float & floatValue) : value(_IQ(floatValue)) {;}
    __fast_inline explicit iq_t(const double & doubleValue) : value(_IQ(doubleValue)) {;}

    __fast_inline iq_t operator+(const iq_t & other) const {
        return iq_t(value + other.value);
    }

    __fast_inline iq_t operator-(const iq_t & other) const {
        return iq_t(value - other.value);
    }

    __fast_inline iq_t operator-() const {
        return iq_t(-value);
    }

    __fast_inline iq_t operator*(const iq_t & other) const {
        return iq_t(_IQmpy(value, other.value));
    }

    __fast_inline iq_t operator/(const iq_t & other) const {
        return iq_t(_IQdiv(value, other.value));
    }

    __fast_inline iq_t& operator+=(const iq_t& other) {
        value = value + other.value;
        return *this;
    }

    __fast_inline iq_t& operator-=(const iq_t& other) {
        value = value - other.value;
        return *this;
    }

    __fast_inline iq_t& operator*=(const iq_t& other) {
        value = _IQmpy(value, other.value);
        return *this;
    }

    __fast_inline iq_t& operator/=(const iq_t& other) {
        value = _IQdiv(value, other.value);
        return *this;
    }

    __fast_inline bool operator==(const iq_t & other) const {
        return value == other.value;
    }

    __fast_inline bool operator!=(const iq_t & other) const {
        return value != other.value;
    }

    __fast_inline bool operator>(const iq_t & other) const {
        return value > other.value;
    }

    __fast_inline bool operator<(const iq_t & other) const {
        return value < other.value;
    }

    __fast_inline bool operator>=(const iq_t & other) const {
        return value >= other.value;
    }

    __fast_inline bool operator<=(const iq_t & other) const {
        return value <= other.value;
    }

    #ifndef STRICT_IQ

    __fast_inline iq_t operator+(const int & other) const {
        return iq_t(value + _IQ(other));
    }

    __fast_inline iq_t operator+(const float & other) const {
        return iq_t(value + _IQ(other));
    }

    __fast_inline iq_t operator+(const double & other) const {
       return iq_t(value + _IQ(other));
    }

    __fast_inline iq_t operator-(const int & other) const {
        return iq_t(value - _IQ(other));
    }

    __fast_inline iq_t operator-(const float & other) const {
        return iq_t(value - _IQ(other));
    }

    __fast_inline iq_t operator-(const double & other) const {
       return iq_t(value - _IQ(other));
    }

    __fast_inline iq_t operator*(const int & other) const {
        return iq_t(value * other);
    }

    __fast_inline iq_t operator*(const float & other) const {
        return iq_t(*this * iq_t(other));
    }

    __fast_inline iq_t operator*(const double & other) const {
        return iq_t(*this * iq_t(other));
    }

    __fast_inline iq_t operator/(const int & other) const {
        return iq_t((value / other));
    }

    __fast_inline iq_t operator/(const float & other) const {
        return iq_t(*this / iq_t(other));
    }

    __fast_inline iq_t operator/(const double & other) const {
        return iq_t(*this / iq_t(other));
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
        *this /= other;
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

    __fast_inline bool operator==(const int & other) const {
        return value == _IQ(other);
    }

    __fast_inline bool operator==(const float & other) const {
        return value == _IQ(other);
    }

    __fast_inline bool operator==(const double & other) const {
        return value == _IQ(other);
    }

    __fast_inline bool operator!=(const int & other) const {
        return value != _IQ(other);
    }

    __fast_inline bool operator!=(const float & other) const {
        return value != _IQ(other);
    }

    __fast_inline bool operator!=(const double & other) const {
        return value != _IQ(other);
    }

    __fast_inline bool operator>(const int & other) const {
        return value > _IQ(other);
    }

    __fast_inline bool operator>(const float & other) const {
        return value > _IQ(other);
    }

    __fast_inline bool operator>(const double & other) const {
        return value > _IQ(other);
    }

    __fast_inline bool operator<(const int & other) const {
        return value < _IQ(other);
    }

    __fast_inline bool operator<(const float & other) const {
        return value < _IQ(other);
    }

    __fast_inline bool operator<(const double & other) const {
        return value < _IQ(other);
    }

    __fast_inline bool operator>=(const int & other) const {
        return value >= _IQ(other);
    }

    __fast_inline bool operator>=(const float & other) const {
        return value >= _IQ(other);
    }

    __fast_inline bool operator>=(const double & other) const {
        return value >= _IQ(other);
    }

    __fast_inline bool operator<=(const int & other) const {
        return value <= _IQ(other);
    }

    __fast_inline bool operator<=(const float & other) const {
        return value <= _IQ(other);
    }

    __fast_inline bool operator<=(const double & other) const {
        return value <= _IQ(other);
    }

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
    
    #ifdef EXTRA_IQ


    #endif
    
    __fast_inline explicit operator bool() const {
        return bool(value);
    }

    __fast_inline explicit operator int() const {
        return _IQint(value);
    }

    __fast_inline explicit operator uint8_t() const {
        return _IQint(value);
    }

    __fast_inline explicit operator int8_t() const {
        return _IQint(value);
    }

    __fast_inline explicit operator uint16_t() const {
        return _IQint(value);
    }

    __fast_inline explicit operator int16_t() const {
        return _IQint(value);
    }

    __fast_inline explicit operator uint32_t() const {
        return _IQint(value);
    }

    __fast_inline explicit operator int32_t() const {
        return _IQint(value);
    }

    __fast_inline explicit operator float() const{
        return _IQtoF(value);
    }

    __fast_inline explicit operator double() const{
        return _IQtoD(value);
    }

    __no_inline explicit operator String() const{
        return String(_IQtoF(value));
    }

    __no_inline String toString(unsigned char decimalPlaces = 2){
        return String(static_cast<float>(*this), decimalPlaces);
    }
};

#ifndef STRICT_IQ

__fast_inline iq_t operator+(int int_v, const iq_t & iq_v) {
	return iq_v + iq_t(int_v);
}

__fast_inline iq_t operator+(float float_v, const iq_t & iq_v) {
	return iq_v + iq_t(float_v);
}

__fast_inline iq_t operator+(double double_v, const iq_t & iq_v) {
	return iq_v + iq_t(double_v);
}

__fast_inline iq_t operator-(int int_v, const iq_t & iq_v) {
	return iq_t(int_v) - iq_v;
}

__fast_inline iq_t operator-(float float_v, const iq_t & iq_v) {
	return iq_t(float_v) - iq_v;
}

__fast_inline iq_t operator-(double double_v, const iq_t & iq_v) {
	return iq_t(double_v) - iq_v;
}

__fast_inline iq_t operator*(int int_v, const iq_t & iq_v) {
	return iq_v * int_v;
}

__fast_inline iq_t operator*(float float_v, const iq_t & iq_v) {
	return iq_v * float_v;
}

__fast_inline iq_t operator*(double double_v, const iq_t & iq_v) {
	return iq_v * double_v;
}

__fast_inline iq_t operator/(int int_v, const iq_t & iq_v) {
	return iq_t(int_v) / iq_v;
}

__fast_inline iq_t operator/(float float_v, const iq_t & iq_v) {
	return iq_t(float_v) / iq_v;
}

__fast_inline iq_t operator/(double double_v, const iq_t & iq_v) {
	return iq_t(double_v) / iq_v;
}

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

__fast_inline iq_t abs(const iq_t & iq) {return iq_t(_IQabs(iq.value));}

__fast_inline bool isnormal(const iq_t & iq){return bool(iq.value);}

__fast_inline bool signbit(const iq_t & iq){return bool(iq.value < 0);}

__fast_inline iq_t sign(const iq_t & iq);

__fast_inline iq_t fmod(const iq_t & a, const iq_t & b){return iq_t((a.value < b.value && (0 <= a.value)) ? a.value : _IQmpy(_IQfrac(_IQdiv(a.value, b.value)), b.value));}


__fast_inline iq_t lerp(const iq_t & x, const iq_t & a, const iq_t & b){return a * (iq_t(1) - x) + b * x;}
__fast_inline iq_t mean(const iq_t & a, const iq_t & b){return iq_t((a.value + b.value) >> 1);}

__fast_inline iq_t frac(const iq_t & iq){return iq_t(_IQfrac(iq.value));}

__fast_inline iq_t floor(const iq_t & iq){return iq_t(iq.value - _IQfrac(iq.value));}

__fast_inline iq_t round(const iq_t & iq){return iq_t((int)_IQint(iq.value + _IQ(0.5)));}

bool is_equal_approx(const iq_t & a,const iq_t & b);

bool is_equal_approx_ratio(const iq_t a, const iq_t & b, iq_t epsilon = iq_t(CMP_EPSILON), iq_t min_epsilon = iq_t(CMP_EPSILON));

#ifdef USE_LOG

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

__fast_inline void s16_to_uni(const int16_t & data, iq_t & qv){
#if GLOBAL_Q >= 16
    qv.value = (data << (GLOBAL_Q - 15)) - ((1 << GLOBAL_Q));
#else
    qv.value = (data >> (17 - GLOBAL_Q)) - ((1 << GLOBAL_Q));
#endif
}

__fast_inline void uni_to_u16(const iq_t & qv, uint16_t & data){
#if GLOBAL_Q >= 16
    data = qv.value >> (GLOBAL_Q - 16);
#else
    data = qv.value << (16 - GLOBAL_Q);
#endif
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
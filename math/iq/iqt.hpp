#ifndef __IQT_HPP__

#define __IQT_HPP__


#include "stdint.h"
#include "defines/comm_inc.h"

#ifdef USE_IQ
#include <IQmath_RV32.h>
#else
#include <cmath>
#endif


class iq_t{

private:
    volatile _iq value = 0;

public:
    __fast_inline explicit iq_t(): value(0){;}
    __fast_inline explicit iq_t(const _iq & iqValue): value(iqValue){;}
    __fast_inline explicit iq_t(const int & intValue) : value(_IQ(static_cast<float>(intValue))) {;}
    __fast_inline explicit iq_t(const float & floatValue) : value(_IQ(floatValue)) {;}
    __fast_inline explicit iq_t(const double & floatValue) : value(_IQ(static_cast<float>(floatValue))) {;}

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
        return !(value == other.value);
    }

    __fast_inline bool operator>(const iq_t & other) const {
        return value > other.value;
    }

    __fast_inline bool operator<(const iq_t & other) const {
        return value < other.value;
    }

    __fast_inline bool operator>=(const iq_t & other) const {
        return value > other.value;
    }

    __fast_inline bool operator<=(const iq_t & other) const {
        return value < other.value;
    }

    __fast_inline explicit operator int() const {
        return _IQint(value);
    }

    __fast_inline explicit operator float() const{
        return _IQtoF(value);
    }

    __fast_inline explicit operator double() const{
        return double(_IQtoF(value));
    }

    __fast_inline _iq getValue() const{
        return value;
    }

};

namespace std
{
    __fast_inline iq_t sin(const iq_t & iq){return iq_t(_IQsin(iq.getValue()));}

    __fast_inline iq_t cos(const iq_t & iq){return iq_t(_IQcos(iq.getValue()));}

    __fast_inline iq_t tan(const iq_t & iq) {return iq_t(_IQsin(iq.getValue()) / _IQcos(iq.getValue()));}

    __fast_inline iq_t asin(const iq_t & iq) {return iq_t(_IQasin(iq.getValue()));}

    __fast_inline iq_t acos(const iq_t & iq) {return iq_t(_IQacos(iq.getValue()));}

    __fast_inline iq_t atan(const iq_t & iq) {return iq_t(_IQatan(iq.getValue()));}

    __fast_inline iq_t atan2(const iq_t & a, const iq_t & b) {return iq_t(_IQatan2(a.getValue(),b.getValue()));}

    __fast_inline iq_t sqrt(const iq_t & iq){return iq_t(_IQsqrt(iq.getValue()));}

    __fast_inline iq_t abs(const iq_t & iq) {return iq_t(_IQabs(iq.getValue()));}

    __fast_inline bool isnormal(const iq_t & iq){return bool(iq.getValue());}

    __fast_inline bool signbit(const iq_t & iq){return bool(iq.getValue() < 0);}

    __fast_inline iq_t fmod(const iq_t & a, const iq_t & b){return iq_t(_IQmpy(_IQfrac(_IQdiv(a.getValue(), b.getValue())), b.getValue()));}

    __fast_inline iq_t mean(const iq_t & a, const iq_t & b){return iq_t((a.getValue() + b.getValue()) >> 1);}

    __fast_inline iq_t frac(const iq_t & iq){return iq_t(_IQfrac(iq.getValue()));}

    __fast_inline iq_t floor(const iq_t & iq){return iq_t(iq.getValue() - _IQfrac(iq.getValue()));}

    #ifdef USE_LOG
    
    __fast_inline iq_t log10(const iq_t & iq) {return iq_t(_IQlog10(iq.getValue()));}

    __fast_inline iq_t log(const iq_t & iq) {return iq_t(_IQdiv(_IQlog10(iq.getValue()), _IQlog10(_IQ(LOGE))));}

    #endif
};

#ifdef USE_IQ
typedef iq_t real_t;
#elif USE_DOUBLE
typedef double real_t;
#else
typedef float real_t;
#endif

#endif
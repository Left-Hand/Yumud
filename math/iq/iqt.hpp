#ifndef __IQT_HPP__

#define __IQT_HPP__

#define USE_IQ
#define USE_LOG

#include <cstdint>
#include "../math/math_defs.h"

#ifdef USE_IQ
#include <IQmath_RV32.h>
#else
#include <cmath>
#endif

class iq_t{

private:
    volatile _iq value = 0;

public:
    explicit iq_t(): value(0){;}
    // explicit iq_t(const iq_t & other): value(other.value) {;}
    explicit iq_t(const _iq & iqValue): value(iqValue){;}
    explicit iq_t(const int & intValue) : value(_IQ(static_cast<float>(intValue))) {;}
    explicit iq_t(const float & floatValue) : value(_IQ(floatValue)) {;}
    explicit iq_t(const double & floatValue) : value(_IQ(static_cast<float>(floatValue))) {;}

    iq_t operator+(const iq_t & other) const {
        return iq_t(value + other.value);
    }

    iq_t operator-(const iq_t & other) const {
        return iq_t(value - other.value);
    }

    iq_t operator-() const {
        return iq_t(-value);
    }

    iq_t operator*(const iq_t & other) const {
        return iq_t(_IQmpy(value, other.value));
    }

    iq_t operator/(const iq_t & other) const {
        return iq_t(_IQdiv(value, other.value));
    }

    iq_t& operator+=(const iq_t& other) {
        value = value + other.value;
        return *this;
    }

    iq_t& operator-=(const iq_t& other) {
        value = value - other.value;
        return *this;
    }

    iq_t& operator*=(const iq_t& other) {
        value = _IQmpy(value, other.value);
        return *this;
    }

    iq_t& operator/=(const iq_t& other) {
        value = _IQdiv(value, other.value);
        return *this;
    }

    bool operator==(const iq_t & other) const {
        return value == other.value;
    }

    bool operator!=(const iq_t & other) const {
        return !(value == other.value);
    }

    bool operator>(const iq_t & other) const {
        return value > other.value;
    }

    bool operator<(const iq_t & other) const {
        return value < other.value;
    }

    bool operator>=(const iq_t & other) const {
        return value > other.value;
    }

    bool operator<=(const iq_t & other) const {
        return value < other.value;
    }

    explicit operator int() const {
        return _IQint(value);
    }

    explicit operator float() const{
        return _IQtoF(value);
    }

    explicit operator double() const{
        return double(_IQtoF(value));
    }

    inline _iq getValue() const{
        return value;
    }

};


namespace std
{
    iq_t sin(const iq_t & iq);

    iq_t cos(const iq_t & iq);

    iq_t tan(const iq_t & iq);

    iq_t asin(const iq_t & iq);

    iq_t acos(const iq_t & iq);

    iq_t atan(const iq_t & iq);

    iq_t atan2(const iq_t & a, const iq_t & b);

    iq_t sqrt(const iq_t & iq);

    iq_t abs(const iq_t & iq);

    bool isnormal(const iq_t & iq);

    bool signbit(const iq_t & iq);

    iq_t fmod(const iq_t & a, const iq_t & b);

    iq_t mean(const iq_t & a, const iq_t & b);

    iq_t frac(const iq_t & iq);

    iq_t floor(const iq_t & iq);

    #ifdef USE_LOG
    
    iq_t log10(const iq_t & iq);

    iq_t log(const iq_t & iq);
    #endif
};
#endif
#ifndef __IQT_HPP__

#define __IQT_HPP__


#include "stdint.h"
#include "defines/comm_inc.h"
#include <IQmath_RV32.h>

#ifndef IQ_VALUE
#define IQ_VALUE(x) (*(_iq *)(&(x)))
#endif

#ifndef IQ_FULL
#define IQ_FULL 0xFFFFFFFFUL
#define IQ_TO_FULL(x) (IQ_VALUE(x) = IQ_FULL)
#endif

class iq_t{

private:
    volatile _iq value = 0;

public:
    __fast_inline iq_t(): value(0){;}
    __fast_inline explicit iq_t(const _iq & iqValue): value(iqValue){;}
    __fast_inline explicit iq_t(const unsigned long & longValue): value(longValue){;}
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
        return iq_t(value ? value * other : 0);
    }

    __fast_inline iq_t operator*(const float & other) const {
        return iq_t(*this * iq_t(other));
    }

    __fast_inline iq_t operator*(const double & other) const {
       return iq_t(*this * iq_t(other));
    }

    __fast_inline iq_t operator/(const int & other) const {
        return iq_t(other ? (value ? value / other : 0) : 0xffffffffUL);
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

    // __fast_inline iq_t x31(){
    //     value = 
    //         (value << 4) + 
    //         (value << 3) +
    //         (value << 2) +
    //         (value << 1) +
    //         value;
    //     return iq_t(IQ_VALUE(*this));
    // }
    
    #endif
    
    __fast_inline explicit operator bool() const {
        return bool(value);
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

namespace std
{
    __fast_inline iq_t sin(const iq_t & iq){return iq_t(_IQsin(IQ_VALUE(iq)));}

    __fast_inline iq_t cos(const iq_t & iq){return iq_t(_IQcos(IQ_VALUE(iq)));}

    __fast_inline iq_t tan(const iq_t & iq) {return iq_t(_IQsin(IQ_VALUE(iq)) / _IQcos(IQ_VALUE(iq)));}

    __fast_inline iq_t asin(const iq_t & iq) {return iq_t(_IQasin(IQ_VALUE(iq)));}

    __fast_inline iq_t acos(const iq_t & iq) {return iq_t(_IQacos(IQ_VALUE(iq)));}

    __fast_inline iq_t atan(const iq_t & iq) {return iq_t(_IQatan(IQ_VALUE(iq)));}

    __fast_inline iq_t atan2(const iq_t & a, const iq_t & b) {return iq_t(_IQatan2(IQ_VALUE(a),IQ_VALUE(b)));}

    __fast_inline iq_t sqrt(const iq_t & iq){return iq_t(_IQsqrt(IQ_VALUE(iq)));}

    __fast_inline iq_t abs(const iq_t & iq) {return iq_t(_IQabs(IQ_VALUE(iq)));}

    __fast_inline bool isnormal(const iq_t & iq){return bool(IQ_VALUE(iq));}

    __fast_inline bool signbit(const iq_t & iq){return bool(IQ_VALUE(iq) < 0);}

    __fast_inline iq_t fmod(const iq_t & a, const iq_t & b){return iq_t((IQ_VALUE(a) < IQ_VALUE(b) && (0 <= IQ_VALUE(a))) ? IQ_VALUE(a) : _IQmpy(_IQfrac(_IQdiv(IQ_VALUE(a), IQ_VALUE(b))), IQ_VALUE(b)));}

    __fast_inline iq_t mean(const iq_t & a, const iq_t & b){return iq_t((IQ_VALUE(a) + IQ_VALUE(b)) >> 1);}

    __fast_inline iq_t frac(const iq_t & iq){return iq_t(_IQfrac(IQ_VALUE(iq)));}

    __fast_inline iq_t floor(const iq_t & iq){return iq_t(IQ_VALUE(iq) - _IQfrac(IQ_VALUE(iq)));}

    __fast_inline iq_t round(const iq_t & iq){return iq_t((int)_IQint(IQ_VALUE(iq) + _IQ(0.5)));}

    __fast_inline bool is_equal_approx_ratio(iq_t a, iq_t b, iq_t epsilon = iq_t(CMP_EPSILON), iq_t min_epsilon = iq_t(CMP_EPSILON));

    #ifdef USE_LOG
    
    __fast_inline iq_t log10(const iq_t & iq) {return iq_t(_IQlog10(IQ_VALUE(iq)));}

    __fast_inline iq_t log(const iq_t & iq) {return iq_t(_IQdiv(_IQlog10(IQ_VALUE(iq)), _IQlog10(_IQ(LOGE))));}

    __fast_inline iq_t exp(const iq_t & iq) {return iq_t(_IQexp(IQ_VALUE(iq)));}

    __fast_inline iq_t exp2(const iq_t & iq) {return iq_t(_IQexp2(IQ_VALUE(iq)));}

    __fast_inline iq_t pow(const iq_t & base, const iq_t & exponent) {return iq_t(_IQexp(_IQmpy(IQ_VALUE(exponent), _IQdiv(_IQlog10(IQ_VALUE(base)), _IQlog10(_IQ(LOGE))))));}
    #endif
};

#if defined(IQ_VALUE)
#undef IQ_VALUE
#endif

#endif
#ifndef __IQT_HPP__

#define __IQT_HPP__


#include "sys/core/platform.h"

#include "dsp/constexprmath/ConstexprMath.hpp"

#if defined(RISCV)
#include "riscv/IQmath_RV32.h"
#endif

#include "sys/math/float/fp32.hpp"

#include <bit>



struct _iq{
    _iq16 value = 0;
    __fast_inline constexpr explicit _iq(const _iq16 _value) : value(_value){;}
    __fast_inline constexpr operator _iq16() const{return value;}
};


struct iq_t;

namespace std{
    template <>
    struct is_arithmetic<iq_t> : std::true_type {};

    template <>
    struct is_floating_point<iq_t> : std::false_type {};
    //sounds funny
}

struct iq_t{
private:
    __fast_inline scexpr _iq float_to_iq(const float fv){
        int32_t d = std::bit_cast<int32_t>(fv);
        int32_t exponent = ((d >> 23) & 0xff);
        uint64_t mantissa = (exponent == 0) ? (0) : ((d & ((1 << 23) - 1)) | (1 << 23));

        uint64_t temp;
        if(exponent == 0 or exponent == 0xff){
            temp = 0;
        }else{
            temp = LSHIFT(mantissa, exponent - 127);
        }

        uint64_t  uresult = RSHIFT(temp, (23 - GLOBAL_Q));
        int32_t result = d > 0 ? uresult : -uresult;

        if((bool(d > 0) ^ bool(result > 0)) or (uresult > (uint64_t)0x80000000)){//OVERFLOW
            if(d > 0){
                result = std::bit_cast<_iq>(0x7FFFFFFF);
            }else{
                result = std::bit_cast<_iq>(0x80000000);
            }
        }

        {
            return std::bit_cast<_iq>(result);
        }
    }
public:
    _iq value;

    __fast_inline constexpr iq_t():value(0){;}

    __fast_inline constexpr iq_t(const _iq & other) : value(other){;}
    __fast_inline explicit constexpr operator _iq() const {return value;}
    __fast_inline constexpr iq_t(_iq && other) : value(other){;}

    template<typename T>
    requires std::is_integral_v<T>
    __fast_inline constexpr iq_t(const T intValue) : value(intValue << GLOBAL_Q) {;}

    #ifdef STRICT_IQ
    __fast_inline consteval explicit iq_t(const float fv):value((std::is_constant_evaluated()) ? float_to_iq(fv) : float_to_iq(fv)){};
    // __fast_inline consteval iq_t(const float fv):value((std::is_constant_evaluated()) ? float_to_iq(fv) : float_to_iq(fv)){};
    #else
    __fast_inline constexpr iq_t(const float fv):value((std::is_constant_evaluated()) ? float_to_iq(fv) : float_to_iq(fv)){};
    #endif

    static __fast_inline constexpr iq_t form (const floating auto fv){iq_t ret; ret.value = float_to_iq(fv); return ret;}

    __fast_inline_constexpr iq_t operator+(const iq_t other) const {
        return iq_t(_iq(value + other.value));
    }

    __fast_inline_constexpr iq_t operator-(const iq_t other) const {
        return iq_t(_iq(value - other.value));
    }

    __fast_inline_constexpr iq_t operator-() const {
        return iq_t(_iq(-value));
    }

    __fast_inline_constexpr iq_t& operator+=(const iq_t other) {
        value = _iq((int32_t)value + (int32_t)other.value);
        return *this;
    }

    __fast_inline_constexpr iq_t& operator-=(const iq_t other) {
        value = _iq(value - other.value);
        return *this;
    }

    __fast_inline_constexpr iq_t& operator*=(const iq_t other) {
        *this = *this * other;
        return *this;
    }

    __fast_inline_constexpr iq_t& operator/=(const iq_t other) {
        *this = *this / other;
        return *this;
    }

    template<integral T>
    __fast_inline_constexpr iq_t operator*(const T other) const {
        return iq_t(_iq(value * other));
    }

    template<integral T>
    __fast_inline_constexpr iq_t operator/(const T other) const {
        return iq_t(_iq((value / other)));
    }


    #ifdef  STRICT_IQ
    
    iq_t operator*(const floating auto other) = delete;
    iq_t operator/(const floating auto other) = delete;
    #endif
    
    __fast_inline_constexpr iq_t operator*(const iq_t other) const {
        return iq_t((_iq)((int64_t)value * (int64_t)other.value >> GLOBAL_Q));
    }

    __fast_inline_constexpr iq_t operator/(const iq_t other) const {
        if (std::is_constant_evaluated()) {
            return iq_t((_iq)((int64_t)value / (int64_t)other.value << GLOBAL_Q));
        }else{
            return iq_t(_iq(_IQdiv(value, other.value)));
        }
    }

    __fast_inline constexpr bool operator==(const iq_t other) const {
        return value == other.value;
    }

    __fast_inline constexpr bool operator!=(const iq_t other) const {
        return value != other.value;
    }

    __fast_inline constexpr bool operator>(const iq_t other) const {
        return value > other.value;
    }

    __fast_inline constexpr bool operator<(const iq_t other) const {
        return value < other.value;
    }

    __fast_inline constexpr bool operator>=(const iq_t other) const {
        return value >= other.value;
    }

    __fast_inline constexpr bool operator<=(const iq_t other) const {
        return value <= other.value;
    }

    __fast_inline_constexpr iq_t operator<<(int shift) const {
        return iq_t(_iq(value << shift));
    }

    __fast_inline_constexpr iq_t operator>>(int shift) const {
        return iq_t(_iq(value >> shift));
    }

    #undef IQ_OPERATOR_TEMPLATE

    __fast_inline_constexpr explicit operator bool() const {
        return bool(value);
    }

    #define IQ_TOINT_TEMPLATE(op)\
    __fast_inline_constexpr explicit operator op() const {\
        return op(_IQint(value));\
    }

    IQ_TOINT_TEMPLATE(int);
    IQ_TOINT_TEMPLATE(int8_t);
    IQ_TOINT_TEMPLATE(int16_t);
    IQ_TOINT_TEMPLATE(int32_t);
    IQ_TOINT_TEMPLATE(uint8_t);
    IQ_TOINT_TEMPLATE(uint16_t);
    IQ_TOINT_TEMPLATE(uint32_t);

    #undef IQ_TOINT_TEMPLATE

    __inline constexpr explicit operator float() const{
        if(std::is_constant_evaluated()){
            return float(this->value) / (1 << GLOBAL_Q);
        }else{
            return _IQtoF(this->value);
        }
    }

    __inline constexpr explicit operator double() const{
        return float(*this);
    }

};

#define IQ_OP_TEMPLATE(type, op)\
__fast_inline_constexpr iq_t operator op (const type val, const iq_t iq_v) {\
	return iq_t(val) op iq_v;\
}\

#define IQ_OPS_TEMPLATE(type)\
IQ_OP_TEMPLATE(type, +)\
IQ_OP_TEMPLATE(type, -)\
IQ_OP_TEMPLATE(type, *)\
IQ_OP_TEMPLATE(type, /)\

IQ_OPS_TEMPLATE(int);
#ifndef STRICT_IQ
#else
#define IQ_OP_DELETE(op)\
iq_t operator op (const float val, const iq_t iq_v) = delete;\

IQ_OP_DELETE(+)
IQ_OP_DELETE(-)
IQ_OP_DELETE(*)
IQ_OP_DELETE(/)

#undef IQ_OP_DELETE

#endif


#define IQ_BINA_TEMPLATE(type, op)\
__fast_inline_constexpr bool operator op (const type val, const iq_t iq_v) {\
	return iq_t(val) op iq_v;\
}\

#define IQ_BINAS_TEMPLATE(type)\
IQ_BINA_TEMPLATE(type, ==)\
IQ_BINA_TEMPLATE(type, !=)\
IQ_BINA_TEMPLATE(type, >)\
IQ_BINA_TEMPLATE(type, >=)\
IQ_BINA_TEMPLATE(type, <)\
IQ_BINA_TEMPLATE(type, <=)\

IQ_BINAS_TEMPLATE(int)
#ifndef STRICT_IQ
#else
#define IQ_BINA_DELETE(op)\
bool operator op (const float val, const iq_t iq_v) = delete;\

IQ_BINA_DELETE(==)
IQ_BINA_DELETE(!=)
IQ_BINA_DELETE(>)
IQ_BINA_DELETE(>=)

#undef IQ_BINA_DELETE
#endif

#undef IQ_OP_TEMPLATE
#undef IQ_OPS_TEMPLATE
#undef IQ_BINA_TEMPLATE
#undef IQ_BINAS_TEMPLATE

using cem = ConstexprMath;

__fast_inline iq_t sinf(const iq_t iq){
    {
        return iq_t(_iq(_IQsin(iq.value)));
    }
}

__fast_inline iq_t cosf(const iq_t iq){
    {
        return iq_t(_iq(_IQcos(iq.value)));
    }
}

__fast_inline iq_t sin(const iq_t iq){return sinf(iq);}

__fast_inline iq_t cos(const iq_t iq){return cosf(iq);}

__fast_inline iq_t tanf(const iq_t iq) {return sin(iq) / cos(iq);}
__fast_inline iq_t tan(const iq_t iq) {return tanf(iq);}

__fast_inline iq_t asin(const iq_t iq) {
    {
        return iq_t(_iq(_IQasin(iq.value)));
    }
}

__fast_inline iq_t acos(const iq_t iq) {
    {
        return iq_t(_iq(_IQacos(iq.value)));
    }
}

__fast_inline iq_t atan(const iq_t iq) {
    {
        return iq_t(_iq(_IQatan(iq.value)));
    }
}

__fast_inline iq_t atan2f(const iq_t a, const iq_t b) {
    {
        return iq_t(_iq(_IQatan2(a.value,b.value)));
    }
}

__fast_inline iq_t atan2(const iq_t a, const iq_t b) {
    return atan2f(a, b);
}

__fast_inline iq_t sqrt(const iq_t iq){
    {
        return iq_t(_iq(_IQsqrt(iq.value)));
    }
}

__fast_inline_constexpr iq_t abs(const iq_t iq){
    if(long(iq.value) > 0){
        return iq;
    }else{
        return -iq;
    }
}

__fast_inline_constexpr bool isnormal(const iq_t iq){return bool(iq.value);}

__fast_inline_constexpr bool signbit(const iq_t iq){return bool(long(iq.value) < 0);}


__fast_inline_constexpr iq_t sign(const iq_t iq){
    if(long(iq.value)){
        if(long(iq.value) > 0){
            return iq_t(1);
        }else{
            return iq_t(-1);
        }
    }else return iq_t(0);
}

__fast_inline_constexpr iq_t fmod(const iq_t a, const iq_t b){return iq_t(_iq(a.value % b.value));}


__fast_inline_constexpr iq_t lerp(const iq_t x, const iq_t a, const iq_t b){return a * (iq_t(1) - x) + b * x;}
__fast_inline_constexpr iq_t mean(const iq_t a, const iq_t b){return iq_t(_iq((a.value + b.value) >> 1));}

__fast_inline_constexpr iq_t frac(const iq_t iq){
    return iq_t(_iq((iq.value) & ((1 << GLOBAL_Q) - 1)));
}

__fast_inline_constexpr iq_t floor(const iq_t iq){return int(iq);}
__fast_inline_constexpr iq_t ceil(const iq_t iq){return (iq > int(iq)) ? int(iq) + 1 : int(iq);}

__fast_inline_constexpr iq_t round(const iq_t iq){return iq_t((int)_IQint(long(iq.value) + _IQ(0.5)));}

bool is_equal_approx(const iq_t a,const iq_t b);

bool is_equal_approx_ratio(const iq_t a, const iq_t b, iq_t epsilon = iq_t(CMP_EPSILON), iq_t min_epsilon = iq_t(CMP_EPSILON));

#ifdef IQ_USE_LOG

__fast_inline iq_t log10(const iq_t iq) {
    // if(std::is_constant_evaluated()){
    //     return iq_t(cem::ln(double(iq)) / cem::ln(10.0));
    // }else
    {
        return iq_t(_iq(_IQlog10(iq.value)));
    }
}

__fast_inline iq_t log(const iq_t iq) {
    // if(std::is_constant_evaluated()){
    //     return iq_t(cem::ln(double(iq)));
    // }else
    {
        return iq_t(_iq(_IQdiv(_IQlog10(iq.value), _IQlog10(_IQ(M_E)))));
    }
}

__fast_inline iq_t exp(const iq_t iq) {
    // if(std::is_constant_evaluated()){
    //     return iq_t(cem::exp(double(iq)));
    // }else
    {
        return iq_t(_iq(_IQexp(iq.value)));
    }
}

__fast_inline iq_t exp2(const iq_t iq) {
    // if(std::is_constant_evaluated()){
    //     return iq_t(cem::pow(2.0, double(iq)));
    // }else
    {
        return iq_t(_iq(_IQexp2(iq.value)));
    }
}

__fast_inline iq_t pow(const iq_t base, const iq_t exponent) {
    // if(std::is_constant_evaluated()){
    //     return iq_t(cem::pow(double(base), double(exponent)));
    // }else
    {
        return iq_t(_iq(_IQexp(_IQmpy(exponent.value, _IQdiv(base.value, _IQlog10(_IQ(LOG_E)))))));
    }
}

#endif

__fast_inline_constexpr void u16_to_uni(const uint16_t data, iq_t & qv){
#if GLOBAL_Q > 16
    qv.value = data << (GLOBAL_Q - 16);
#elif(GLOBAL_Q < 16)
    qv.value = data >> (16 - GLOBAL_Q);
#else
    qv.value = _iq(data);
#endif

}


__fast_inline_constexpr void u32_to_uni(const uint32_t data, iq_t & qv){
#if GLOBAL_Q > 16
    qv.value = data << (GLOBAL_Q - 16);
#elif(GLOBAL_Q < 16)
    qv.value = data >> (16 - GLOBAL_Q);
#else
    qv.value = _iq(data);
#endif

}

__fast_inline_constexpr void s16_to_uni(const int16_t data, iq_t & qv){
    qv.value = data > 0 ? _iq(data) : _iq(-(_iq(-data)));
}

__fast_inline_constexpr void uni_to_u16(const iq_t qv, uint16_t & data){
#if GLOBAL_Q >= 16
    data = qv.value >> (GLOBAL_Q - 16);
#else
    data = qv.value << (16 - GLOBAL_Q);
#endif
    if(data == 0 && long(qv.value) != 0) data = 0xffff;
}


namespace std{
    template<>
    class numeric_limits<iq_t> {
    public:
        __fast_inline_constexpr static iq_t infinity() noexcept {return iq_t(_iq(0x80000000));}
        __fast_inline_constexpr static iq_t lowest() noexcept {return iq_t(_iq(0x7FFFFFFF));}

        __fast_inline_constexpr static iq_t min() noexcept {return iq_t(_iq(0x80000000));}
        __fast_inline_constexpr static iq_t max() noexcept {return iq_t(_iq(0x7FFFFFFF));}
    };

    #ifndef STRICT_IQ
    typedef std::common_type<iq_t, float>::type iq_t;
    typedef std::common_type<iq_t, double>::type iq_t;

    typedef std::common_type<float, iq_t>::type iq_t;
    typedef std::common_type<double, iq_t>::type iq_t;
    #endif

    typedef std::common_type<iq_t, int>::type iq_t;
    typedef std::common_type<int, iq_t>::type iq_t;

    __fast_inline iq_t sinf(const iq_t iq){return ::sinf(iq);}
    __fast_inline iq_t cosf(const iq_t iq){return ::cosf(iq);}
    __fast_inline iq_t sin(const iq_t iq){return ::sin(iq);}
    __fast_inline iq_t cos(const iq_t iq){return ::cos(iq);}
    __fast_inline iq_t tanf(const iq_t iq){return ::tanf(iq);}
    __fast_inline iq_t tan(const iq_t iq){return ::tan(iq);}
    __fast_inline iq_t asinf(const iq_t iq){return ::asin(iq);}
    __fast_inline iq_t asin(const iq_t iq){return ::asin(iq);}
    __fast_inline iq_t acos(const iq_t iq){return ::acos(iq);}
    __fast_inline iq_t atan(const iq_t iq){return ::atan(iq);}
    __fast_inline iq_t atan2f(const iq_t a, const iq_t b){return ::atan2f(a,b);}
    __fast_inline iq_t atan2(const iq_t a, const iq_t b){return ::atan2(a,b);}
    __fast_inline iq_t sqrt(const iq_t iq){return ::sqrt(iq);}
    __fast_inline iq_t abs(const iq_t iq){return ::abs(iq);}
    __fast_inline bool isnormal(const iq_t iq){return ::isnormal(iq);}
    __fast_inline bool signbit(const iq_t iq){return ::signbit(iq);}
    __fast_inline iq_t fmod(const iq_t a, const iq_t b){return ::fmod(a, b);}
    __fast_inline iq_t mean(const iq_t a, const iq_t b){return ::mean(a, b);}
    __fast_inline iq_t frac(const iq_t iq){return ::frac(iq);}
    __fast_inline iq_t floor(const iq_t iq){return ::floor(iq);}
    __fast_inline iq_t ceil(const iq_t iq){return ::ceil(iq);}

    #ifdef IQ_USE_LOG
    __fast_inline iq_t log10(const iq_t iq){return ::log10(iq);}
    __fast_inline iq_t log(const iq_t iq){return ::log(iq);}
    #endif
}

#endif
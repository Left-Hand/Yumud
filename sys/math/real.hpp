#pragma once

#include "sys/core/platform.h"

#if (!defined(USE_IQ)) &&  (!defined(USE_STD_MATH))
#define USE_STD_MATH
#endif

#ifdef USE_IQ
#include "iq/iq_t.hpp"
#endif

#if defined(USE_STDMATH)

// #ifdef __cplusplus
#include <cmath>
// #else
// #include "math.h"
// #endif
#include "math.h"


#else
#include "dsp/floatlib/floatlib.h"
#endif

#ifdef USE_IQ
typedef iq_t real_t;
#elif defined(USE_DOUBLE)
typedef double real_t;
#else
typedef float real_t;
#endif

scexpr real_t pi_4 = real_t(PI/4);
scexpr real_t pi_2 = real_t(PI/2);
scexpr real_t pi = real_t(PI);
scexpr real_t tau = real_t(TAU);


consteval real_t operator"" _r(long double x){
    return real_t(x);
}


__fast_inline constexpr int mean(const int a, const int b){
    return ((a+b) >> 1);
}

template<floating T>
__fast_inline constexpr T mean(const T & a, const T & b){
    return (a+b) / 2.0f;
}

template<floating T>
__fast_inline constexpr T frac(const T fv){
    return (fv - T(int(fv)));
}

template<floating T>
__fast_inline constexpr T round(const T x)
{
    return (int)(x+0.5f);
}


template<floating T>
__fast_inline constexpr bool is_equal_approx(const T & a, const T & b) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a == b) {
        return true;
    }
    // Then check for approximate equality.
    auto tolerance = CMP_EPSILON * ABS(a);
    if (tolerance < CMP_EPSILON) {
        tolerance = CMP_EPSILON;
    }
    return ABS(a - b) < tolerance;
}


template<floating T>
__fast_inline constexpr bool is_equal_approx_ratio(const T a, const T b, const T epsilon, const T min_epsilon){
    auto diff = ABS(a - b);
    if (diff == 0.0 || diff < min_epsilon) {
        return true;
    }
    auto avg_size = (ABS(a) + ABS(b)) / 2.0;
    diff /= avg_size;
    return diff < epsilon;
}

__fast_inline constexpr float sign(const float fv){
    if(fv > 0.0f) return 1.0f;
    else if(fv < 0.0f) return -1.0f;
    return 0.0f;
}

__fast_inline constexpr double sign(const double dv){
    if(dv > 0.0) return 1.0;
    else if(dv < 0.0) return -1.0;
    return 0.0;
}

template<arithmetic T>
__fast_inline T fposmodp(T p_x, T p_y) {
    T value = fmod(p_x, p_y);
    if (value < 0) {
        value += p_y;
    }
    return value;
}

template<integral T>
__fast_inline T sign(const T val){return val == 0 ? 0 : (val < 0 ? -1 : 1);}

__fast_inline void u16_to_uni(const uint16_t data, float & fv){
    fv = (float)data / 65535;
}

__fast_inline void u16_to_uni(const uint16_t data, double & dv){
    dv = (float)data / 65535;
}

__fast_inline constexpr int warp_mod(const int x, const int y){
    int ret = x % y;
    if(ret < 0) ret += y;
    return ret;
}

template<floating T>
__fast_inline T powfi(const T base, const int exponent) {
    if(0 == exponent) {
        return T(1);
    }else if(1 == exponent){
        return base;
    }else{
        T ret;
        if(1 < exponent){
            for(size_t i = 1; i < exponent; ++i){
                ret *= base;
            }
        }else{
            for(size_t i = 1; i < -exponent; ++i){
                ret /= base;
            }
        }
        return ret;
    }
}

template<floating T>
__fast_inline T powi(const T base, const int exponent) {
    return powi(base, exponent);
}

template<arithmetic T>
__fast_inline T square(const T x) {
    return x * x;
}

#include "real.ipp"


#ifndef __REAL_HPP__

#define __REAL_HPP__

#include "src/platform.h"

#ifdef USE_IQ
#include "iq/iqt.hpp"
#endif

#if defined(USE_STDMATH)

// #ifdef __cplusplus
#include <cmath>
// #else
// #include "math.h"
// #endif
#include "math.h"


#else
#include "../dsp/floatlib/floatlib.h"
#endif

#ifdef USE_IQ
typedef iq_t real_t;
#elif defined(USE_DOUBLE)
typedef double real_t;
#else
typedef float real_t;
#endif


__fast_inline int mean(const int & a, const int & b);
__fast_inline float mean(const float & a, const float & b);
__fast_inline double mean(const double & a, const double & b);

__fast_inline float frac(const float & fv);
__fast_inline double frac(const double & dv);

__fast_inline bool is_equal_approx(const float & a,const float & b);
__fast_inline bool is_equal_approx_ratio(const float a, const float & b, float epsilon = float(CMP_EPSILON), float min_epsilon = float(CMP_EPSILON));
__fast_inline bool is_equal_approx(const double & a,const double & b);
__fast_inline bool is_equal_approx_ratio(const double a, const double & b, double epsilon = double(CMP_EPSILON), double min_epsilon = double(CMP_EPSILON));

__fast_inline float sign(const float & fv);
__fast_inline double sign(const double & dv);

__fast_inline void u16_to_uni(const uint16_t & data, float & fv){
    fv = (float)data / 65535;
}

__fast_inline void u16_to_uni(const uint16_t & data, double & dv){
    dv = (float)data / 65535;
}

__fast_inline int warp_mod(const int x, const int y){
    int ret = x % y;
    if(ret < 0) ret += y;
    return ret;
}

#endif
#ifndef __REAL_HPP__

#define __REAL_HPP__

#include "defines/comm_inc.h"

#ifdef USE_IQ
#include "iq/iqt.hpp"
#endif

#if defined(USE_STDMATH)

#ifdef __cplusplus
#include <cmath>
#else
#include "math.h"
#endif

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

namespace std {
    int mean(const int & a, const int & b);
    float mean(const float & a, const float & b);
    double mean(const double & a, const double & b);

    float frac(const float & fv);
    double frac(const double & dv);
    
    // float abs(float fv);
    // double abs(double dv);
}


#endif
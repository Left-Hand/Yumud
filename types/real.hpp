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
    
	bool is_equal_approx(const float & a,const float & b);
    bool is_equal_approx_ratio(const float a, const float & b, float epsilon = float(CMP_EPSILON), float min_epsilon = float(CMP_EPSILON));
	bool is_equal_approx(const double & a,const double & b);
    bool is_equal_approx_ratio(const double a, const double & b, double epsilon = double(CMP_EPSILON), double min_epsilon = double(CMP_EPSILON));

}


#endif
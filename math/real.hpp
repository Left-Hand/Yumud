#ifndef __REAL_HPP__

#define __REAL_HPP__

#include "floatlib/fastsqrt.h"
#include "iq/iqt.hpp"

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
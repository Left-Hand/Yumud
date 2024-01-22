#ifndef __REAL_HPP__

#define __REAL_HPP__


namespace std {
    int mean(const int & a, const int & b);
    float mean(const float & a, const float & b);
    double mean(const double & a, const double & b);

    float frac(const float & fv);
    double frac(const double & dv);
}


#include "iq/iqt.hpp"


#ifdef USE_IQ
typedef iq_t real_t;
#else
typedef float real_t;
#endif


#endif
#include "real.hpp"

namespace std {
    int mean(const int & a, const int & b){
        return ((a+b) >> 1);
    }

    float mean(const float & a, const float & b){
        return (a+b) / 2.0f;
    }
    
    double mean(const double & a, const double & b){
        return (a+b) / 2.0;
    }

    float frac(const float & fv){
        return (fv - float(int(fv)));
    }

    double frac(const double & dv){
        return (dv - double(int(dv)));
    }
}
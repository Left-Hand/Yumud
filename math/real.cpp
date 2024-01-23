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

    // float fabs(float fv){
	// 	union {
	// 		double f;
	// 		uint64_t i;
	// 	} u;
	// 	u.f = fv;
	// 	u.i &= (uint32_t)2147483647l;
	// 	return u.fv;
    // }

    // double abs(double dv){
	// 	union {
	// 		double d;
	// 		uint64_t i;
	// 	} u;
	// 	u.d = dv;
	// 	u.i &= (uint64_t)9223372036854775807ll;
	// 	return u.dv;
    // }
}
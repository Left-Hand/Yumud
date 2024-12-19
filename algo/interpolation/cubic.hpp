#pragma once

#include "interpolation.hpp"


namespace ymd::intp{

class CubicInterpolation:public Interpolation{
protected:
    real_t _a;
    real_t _b;
    real_t _c;
    real_t _d;
public:
    CubicInterpolation(const Vector2 & ha,const Vector2 & hb):
        _a(ha.x),_b(ha.y),_c(hb.x),_d(hb.y){
            // scexpr double epsilon = 0.001;
            // scexpr real_t min_param_a = real_t(0 + epsilon);
            // scexpr real_t max_param_a = real_t(1 - epsilon);
            // scexpr real_t min_param_b = real_t(0 + epsilon);
            // scexpr real_t max_param_b = real_t(1 - epsilon);
            // a = CLAMP(a, min_param_a, max_param_a);
            // b = CLAMP(b, min_param_b, max_param_b);
    }
protected:
    template<arithmetic T>
    scexpr T slopeFromT (const T k,const T A,const T B,const T C){
        T dtdx = T(1)/(3*A*k*k + 2*B*k + C); 
        return dtdx;
    }

    template<arithmetic T>
    scexpr T xFromT (const T k, const T A, const T B, const T C, const T D){
        T x = A*(k*k*k) + B*(k*k) + C*k + D;
        return x;
    }

    template<arithmetic T>
    scexpr T yFromT (const T k, const T E, const T F, const T G, const T H){
        T y = E*(k*k*k) + F*(k*k) + G*k + H;
        return y;
    }
    static real_t forward(const Vector2 & a,const Vector2 & b, const real_t x);
public:

    real_t forward(const real_t x) const override;
};

class NearCubicInterpolation : public CubicInterpolation{
    using CubicInterpolation::CubicInterpolation;
protected:
    template<arithmetic T>
    scexpr T B0 (T k){
        return (1-k)*(1-k)*(1-k);
    }

    template<arithmetic T>
    scexpr T B1 (T k){
        return  3*k* (1-k)*(1-k);
    }

    template<arithmetic T>
    scexpr T B2 (T k){
        return 3*k*k* (1-k);
    }

    template<arithmetic T>
    scexpr T B3 (T k){
        return k*k*k;
    }

    template<arithmetic T>
    scexpr T  findx (T k, T x0, T x1, T x2, T x3){
        return x0*B0(k) + x1*B1(k) + x2*B2(k) + x3*B3(k);
    }

    template<arithmetic T>
    scexpr T  findy (T k, T y0, T y1, T y2, T y3){
        return y0*B0(k) + y1*B1(k) + y2*B2(k) + y3*B3(k);
    }
protected:
    static real_t forward(const Vector2 & from,const Vector2 & to, const real_t x);
public:
    real_t forward(const real_t x) const override;

};

}
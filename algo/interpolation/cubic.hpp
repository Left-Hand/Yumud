#pragma once

#include "interpolation.hpp"


namespace ymd{

class CubicInterpolation:public Interpolation{

protected:
    template<arithmetic T>
    static T slopeFromT (const T k,const T A,const T B,const T C){
        T dtdx = T(1)/(3*A*k*k + 2*B*k + C); 
        return dtdx;
    }

    template<arithmetic T>
    static T xFromT (const T k, const T A, const T B, const T C, const T D){
        T x = A*(k*k*k) + B*(k*k) + C*k + D;
        return x;
    }

    template<arithmetic T>
    static T yFromT (const T k, const T E, const T F, const T G, const T H){
        T y = E*(k*k*k) + F*(k*k) + G*k + H;
        return y;
    }

public:
    static real_t forward(const Vector2 & a,const Vector2 & b, const real_t x);
};

class NearCubicInterpolation : public CubicInterpolation{
protected:
    template<arithmetic T>
    static T B0 (T k){
        return (1-k)*(1-k)*(1-k);
    }

    template<arithmetic T>
    static T B1 (T k){
        return  3*k* (1-k)*(1-k);
    }

    template<arithmetic T>
    static T B2 (T k){
        return 3*k*k* (1-k);
    }

    template<arithmetic T>
    static T B3 (T k){
        return k*k*k;
    }

    template<arithmetic T>
    static T  findx (T k, T x0, T x1, T x2, T x3){
        return x0*B0(k) + x1*B1(k) + x2*B2(k) + x3*B3(k);
    }

    template<arithmetic T>
    static T  findy (T k, T y0, T y1, T y2, T y3){
        return y0*B0(k) + y1*B1(k) + y2*B2(k) + y3*B3(k);
    }
public:
    static real_t forward(const Vector2 & from,const Vector2 & to, const real_t x);
};

}
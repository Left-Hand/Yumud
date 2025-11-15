#pragma once

#include "interpolation.hpp"


namespace ymd::intp{

class CubicInterpolation:public Interpolation{
protected:
    iq16 _a;
    iq16 _b;
    iq16 _c;
    iq16 _d;
public:
    CubicInterpolation(const Vec2<iq16> & ha,const Vec2<iq16> & hb):
        _a(ha.x),_b(ha.y),_c(hb.x),_d(hb.y){
            // static constexpr double epsilon = 0.001;
            // static constexpr iq16 min_param_a = iq16(0 + epsilon);
            // static constexpr iq16 max_param_a = iq16(1 - epsilon);
            // static constexpr iq16 min_param_b = iq16(0 + epsilon);
            // static constexpr iq16 max_param_b = iq16(1 - epsilon);
            // a = CLAMP(a, min_param_a, max_param_a);
            // b = CLAMP(b, min_param_b, max_param_b);
    }
protected:
    template<arithmetic T>
    static constexpr T slopeFromT (const T k,const T A,const T B,const T C){
        T dtdx = T(1)/(3*A*k*k + 2*B*k + C); 
        return dtdx;
    }

    template<arithmetic T>
    static constexpr T xFromT (const T k, const T A, const T B, const T C, const T D){
        T x = A*(k*k*k) + B*(k*k) + C*k + D;
        return x;
    }

    template<arithmetic T>
    static constexpr T yFromT (const T k, const T E, const T F, const T G, const T H){
        T y = E*(k*k*k) + F*(k*k) + G*k + H;
        return y;
    }
public:
    static iq16 forward(const Vec2<iq16> & a,const Vec2<iq16> & b, const iq16 x);

    iq16 forward(const iq16 x) const override;
};

class NearCubicInterpolation : public CubicInterpolation{
    using CubicInterpolation::CubicInterpolation;
protected:
    template<arithmetic T>
    static constexpr T B0 (T k){
        return (1-k)*(1-k)*(1-k);
    }

    template<arithmetic T>
    static constexpr T B1 (T k){
        return  3*k* (1-k)*(1-k);
    }

    template<arithmetic T>
    static constexpr T B2 (T k){
        return 3*k*k* (1-k);
    }

    template<arithmetic T>
    static constexpr T B3 (T k){
        return k*k*k;
    }

    template<arithmetic T>
    static constexpr T  findx (T k, T x0, T x1, T x2, T x3){
        return x0*B0(k) + x1*B1(k) + x2*B2(k) + x3*B3(k);
    }

    template<arithmetic T>
    static constexpr T  findy (T k, T y0, T y1, T y2, T y3){
        return y0*B0(k) + y1*B1(k) + y2*B2(k) + y3*B3(k);
    }
protected:
public:
    static iq16 forward(const Vec2<iq16> & from,const Vec2<iq16> & to, const iq16 x);
    iq16 forward(const iq16 x) const override;

};

}
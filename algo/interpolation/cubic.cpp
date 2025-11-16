#include "cubic.hpp"

using namespace ymd;
using namespace ymd::intp;

iq16 CubicInterpolation::forward(const iq16 x) const {
    return forward({_a, _b}, {_c, _d}, x);
}

iq16 CubicInterpolation::forward(
        const Vec2<iq16> & a,
        const Vec2<iq16> & b, 
        const iq16 x){

    static constexpr iq16 y0a = 0; // initial y
    static constexpr iq16 x0a = 0; // initial x 
    const iq16 y1a = a.y;    // 1st influence y   
    const iq16 x1a = a.x;    // 1st influence x 
    const iq16 y2a = b.y;    // 2nd influence y
    const iq16 x2a = b.x;    // 2nd influence x
    static constexpr iq16 y3a = 1; // final y 
    static constexpr iq16 x3a = 1; // final x 

    const iq16 A =   x3a - 3 * x2a + 3 *x1a - x0a;
    const iq16 B = 3*x2a - 6*x1a + 3 *x0a;
    const iq16 C = 3*x1a - 3*x0a;   
    const iq16 D =   x0a;

    const iq16 E =   y3a - 3*y2a + 3*y1a - y0a;    
    const iq16 F = 3*y2a - 6*y1a + 3*y0a;             
    const iq16 G = 3*y1a - 3*y0a;             
    const iq16 H =   y0a;

    iq16 currentt = x;
    for (size_t i=0; i < 5; i++){
        iq16 currentx = xFromT (currentt, A,B,C,D); 
        iq16 currentslope = slopeFromT (currentt, A,B,C);
        currentt -= (currentx - x)*(currentslope);
        currentt = CLAMP(currentt,iq16(0),iq16(1)); 
    } 

    return yFromT (currentt,  E,F,G,H);
}


iq16 NearCubicInterpolation::forward(const iq16 x) const {
    return forward({_a, _b}, {_c, _d}, x);
}


iq16 NearCubicInterpolation::forward(
        const Vec2<iq16> & from,
        const Vec2<iq16> & to, 
        const iq16 x){
    auto [a,b] = from;
    const auto [c,d] = to;
    iq16 y = 0;

    static constexpr iq16 epsilon = iq16(0.001);
    static constexpr iq16 min_param_a = iq16(0 + epsilon);
    static constexpr iq16 max_param_a = iq16(1 - epsilon);
    static constexpr iq16 min_param_b = iq16(0 + epsilon);
    static constexpr iq16 max_param_b = iq16(1 - epsilon);
    a = CLAMP(a, min_param_a, max_param_a);
    b = CLAMP(b, min_param_b, max_param_b);

    static constexpr iq16 x0 = 0;  
    static constexpr iq16 y0 = 0;
    iq16 x4 = a;  
    iq16 y4 = b;
    iq16 x5 = c;  
    iq16 y5 = d;
    static constexpr iq16 x3 = 1;  
    static constexpr iq16 y3 = 1;
    iq16 x1 = 0;
    iq16 y1 = 0;
    iq16 x2 = 0;
    iq16 y2 = 0; // to be solved.

    // arbitrary but reasonable 
    // t-values for interior control points
    static constexpr iq16 t1 = iq16(0.3);
    static constexpr iq16 t2 = 1 - t1;

    iq16 B0t1 = B0(t1);
    iq16 B1t1 = B1(t1);
    iq16 B2t1 = B2(t1);
    iq16 B3t1 = B3(t1);
    iq16 B0t2 = B0(t2);
    iq16 B1t2 = B1(t2);
    iq16 B2t2 = B2(t2);
    iq16 B3t2 = B3(t2);

    iq16 ccx = x4 - x0*B0t1 - x3*B3t1;
    iq16 ccy = y4 - y0*B0t1 - y3*B3t1;
    iq16 ffx = x5 - x0*B0t2 - x3*B3t2;
    iq16 ffy = y5 - y0*B0t2 - y3*B3t2;

    x2 = (ccx - (ffx*B1t1)/B1t2) / (B2t1 - (B1t1*B2t2)/B1t2);
    y2 = (ccy - (ffy*B1t1)/B1t2) / (B2t1 - (B1t1*B2t2)/B1t2);
    x1 = (ccx - x2*B2t1) / B1t1;
    y1 = (ccy - y2*B2t1) / B1t1;

    x1 = CLAMP(x1, + epsilon, 1 - epsilon);
    x2 = CLAMP(x2, + epsilon, 1 - epsilon);

    // Note that this function also requires cubicBezier()!
    y = CubicInterpolation::forward({x1,y1}, {x2,y2}, x);
    y = MAX(0, MIN(1, y));
    return y;
}



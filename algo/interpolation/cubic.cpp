#include "cubic.hpp"

real_t CubicInterpolation::forward(const Vector2 & a,const Vector2 & b, const real_t x){

    scexpr real_t y0a = 0; // initial y
    scexpr real_t x0a = 0; // initial x 
    real_t y1a = a.y;    // 1st influence y   
    real_t x1a = a.x;    // 1st influence x 
    real_t y2a = b.y;    // 2nd influence y
    real_t x2a = b.x;    // 2nd influence x
    scexpr real_t y3a = 1; // final y 
    scexpr real_t x3a = 1; // final x 

    real_t A =   x3a - 3 * x2a + 3 *x1a - x0a;
    real_t B = 3*x2a - 6*x1a + 3 *x0a;
    real_t C = 3*x1a - 3*x0a;   
    real_t D =   x0a;

    real_t E =   y3a - 3*y2a + 3*y1a - y0a;    
    real_t F = 3*y2a - 6*y1a + 3*y0a;             
    real_t G = 3*y1a - 3*y0a;             
    real_t H =   y0a;

    real_t currentt = x;
    for (int i=0; i < 5; i++){
        real_t currentx = xFromT (currentt, A,B,C,D); 
        real_t currentslope = slopeFromT (currentt, A,B,C);
        currentt -= (currentx - x)*(currentslope);
        currentt = CLAMP(currentt,real_t(0),real_t(1)); 
    } 

    real_t y = yFromT (currentt,  E,F,G,H);
    return y;
}

real_t NearCubicInterpolation::forward(const Vector2 & from,const Vector2 & to, const real_t x){
    auto [a,b] = from;
    auto [c,d] = to;
    real_t y = 0;
    scexpr real_t epsilon = real_t(0.001);
    real_t min_param_a = 0 + epsilon;
    real_t max_param_a = 1 - epsilon;
    real_t min_param_b = 0 + epsilon;
    real_t max_param_b = 1 - epsilon;
    a = MAX(min_param_a, MIN(max_param_a, a));
    b = MAX(min_param_b, MIN(max_param_b, b));

    real_t x0 = 0;  
    real_t y0 = 0;
    real_t x4 = a;  
    real_t y4 = b;
    real_t x5 = c;  
    real_t y5 = d;
    real_t x3 = 1;  
    real_t y3 = 1;
    real_t x1,y1,x2,y2; // to be solved.

    // arbitrary but reasonable 
    // t-values for interior control points
    real_t t1 = real_t(0.3);
    real_t t2 = real_t(0.7);

    real_t B0t1 = B0(t1);
    real_t B1t1 = B1(t1);
    real_t B2t1 = B2(t1);
    real_t B3t1 = B3(t1);
    real_t B0t2 = B0(t2);
    real_t B1t2 = B1(t2);
    real_t B2t2 = B2(t2);
    real_t B3t2 = B3(t2);

    real_t ccx = x4 - x0*B0t1 - x3*B3t1;
    real_t ccy = y4 - y0*B0t1 - y3*B3t1;
    real_t ffx = x5 - x0*B0t2 - x3*B3t2;
    real_t ffy = y5 - y0*B0t2 - y3*B3t2;

    x2 = (ccx - (ffx*B1t1)/B1t2) / (B2t1 - (B1t1*B2t2)/B1t2);
    y2 = (ccy - (ffy*B1t1)/B1t2) / (B2t1 - (B1t1*B2t2)/B1t2);
    x1 = (ccx - x2*B2t1) / B1t1;
    y1 = (ccy - y2*B2t1) / B1t1;

    x1 = MAX(0+epsilon, MIN(1-epsilon, x1));
    x2 = MAX(0+epsilon, MIN(1-epsilon, x2));

    // Note that this function also requires cubicBezier()!
    y = CubicInterpolation::forward({x1,y1}, {x2,y2}, x);
    y = MAX(0, MIN(1, y));
    return y;
}



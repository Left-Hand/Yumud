#include "quadratic.hpp"

using namespace ymd::intp;

real_t QuadraticInterpolation::forward(const real_t x){
  // adapted from BEZMATH.PS (1993)
  // by Don Lancaster, SYNERGETICS Inc. 
  // http://www.tinaja.com/text/bezmath.html
    auto [a,b] = _handle;
    real_t epsilon = real_t(0.00001);
    a = MAX(0, MIN(1, a)); 
    b = MAX(0, MIN(1, b)); 

    if (a == real_t(0.5)){
        a += epsilon;
    }
    // iq_t a = 1;
    // solve t from x (an inverse operation)
    real_t om2a = 1 - 2*a;
    real_t k = (sqrt(a*a + om2a*x) - a)/om2a;
    real_t y = (1-2*b)*(k*k) + (2*b)*k;
    return y;
}
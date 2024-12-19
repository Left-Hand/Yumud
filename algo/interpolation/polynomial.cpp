#include "polynomial.hpp"

using namespace ymd::intp;

real_t CosineInterpolation::forward(real_t x) const {
    real_t x2 = x*x;
    real_t x4 = x2*x2;
    real_t x6 = x4*x2;
    
    scexpr real_t fa = real_t( 4.0/9.0);
    scexpr real_t fb = real_t(17.0/9.0);
    scexpr real_t fc = real_t(22.0/9.0);
    
    real_t y = fa*x6 - fb*x4 + fc*x2;
    return y;
}


std::tuple<real_t, real_t> SeatInterpolation::get_ab(const Vector2 & handle){

    scexpr double epsilon = 0.001;
    scexpr real_t min_param_a = real_t(0.0 + epsilon);
    scexpr real_t max_param_a = real_t(1.0 - epsilon);
    scexpr real_t min_param_b = real_t(0.0);
    scexpr real_t max_param_b = real_t(1.0);

    auto [a,b] = handle;

    a = MIN(max_param_a, MAX(min_param_a, a));  
    b = MIN(max_param_b, MAX(min_param_b, b)); 
    return std::make_tuple(a,b);
}

real_t SeatInterpolation::forward(const real_t x) const {
    real_t y = 0;
    if (x <= a){
        y = b - b*pow(1-x/a, 3);
    } else {
        y = b + (1-b)*pow((x-a)/(1-a), 3);
    }
    return y;
}

real_t SeatLineInterpolation::forward(const real_t x) const {
    real_t y = 0;
    if (x<=a){
        y = b*x + (1-b)*a*(1-pow(1-x/a, real_t(3)));
    } else {
        y = b*x + (1-b)*(a + (1-a)*pow((x-a)/(1-a), real_t(3)));
    }
    return y;
}

real_t SeatOddInterpolation::forward(const real_t x) const {
    // auto [a,b] = get_ab(handle);
    // int p = 2*n + 1;
    real_t y = 0;
    if (x <= a){
        y = b - b*pow(1-x/a, p);
    } else {
        y = b + (1-b)*pow((x-a)/(1-a), p);
    }
    return y;
}

real_t SymmetricInterpolation::forward(const real_t x) const {
    real_t y = 0;
    if (+_n%2 == 0){ 
        // even polynomial
        if (x<=real_t(0.5)){
        y = pow(2*x, _n)/2;
        } else {
        y = 1 - pow(2*(x-1), _n)/2;
        }
    } 
    
    else { 
        // odd polynomial
        if (x<=real_t(0.5)){
        y = pow(2*x, +_n)/2;
        } else {
        y = 1 + pow(2*(x-1), _n)/2;
        }
    }

    return y;
}



real_t QuadraticSeatInterpolation::forward(const real_t x) const {

    // auto [a,b] = get_ab(handle);
    
    real_t A = (1-b)/(1-a) - (b/a);
    real_t B = (A*(a*a)-b)/a;
    real_t y = A*(x*x) - B*(x);
    y = MIN(1,MAX(0,y)); 
    
    return y;
}
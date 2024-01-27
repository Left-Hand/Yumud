#ifndef __KALMANFILTER_HPP__

#define __KALMANFILTER_HPP__

#include "../types/real.hpp"

class KalmanFilter {

private:
    real_t r;
    real_t q;

    real_t x_last;
    real_t p_last;

    real_t p_mid;
    real_t kg;

    bool init = false;

public:
    KalmanFilter(real_t _r = real_t(1), real_t _q = real_t(1)):r(_r), q(_q){;}
    
    const real_t update(const real_t & x);
    const real_t predict(const real_t & x);
};

#endif

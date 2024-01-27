#ifndef __KALMANFILTER_HPP__

#define __KALMANFILTER_HPP__

#include "../types/real.hpp"

class KalmanFilter {
    public:
        real_t R = real_t(0.02f);
        real_t Q = real_t(1.0f);
        real_t A = real_t(1.0f);
        real_t B = real_t(1.0f);
        real_t C = real_t(1.0f);
        real_t cov;

        real_t x; // Signal without noise
        bool init = false;
        // KalmanFilter(){;}
        KalmanFilter(real_t r, real_t q = real_t(1.0f),real_t a = real_t(1.0f),real_t b = real_t(1.0f),real_t c = real_t(1.0f));
        KalmanFilter(real_t r = real_t(1.0f)):R(r){;}
        
        const real_t update(const real_t & z);
        const real_t predict();
        const real_t uncertainty();
        void offset(const real_t & ofs){x += ofs;}

};

#endif

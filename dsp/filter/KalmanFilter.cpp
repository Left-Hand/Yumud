#include "KalmanFilter.hpp"

const real_t KalmanFilter::update(const real_t & x) {
    if (!init) {
        x_last = x;
        p_last = real_t(0);
        init = true;
    }else {
        x_last = predict(x);
        p_last = (1-kg)*p_mid;
    }
    return x_last;
}

const real_t KalmanFilter::predict(const real_t & x) {
    p_mid = p_last + q;
    kg = p_mid / (p_mid + r);
    return (x_last + kg*(x - x_last));
}
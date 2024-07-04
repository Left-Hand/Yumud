#ifndef __KALMAN_FILTER_HPP__

#define __KALMAN_FILTER_HPP__

#include "real.hpp"

template<typename real>
requires std::is_arithmetic_v<real>
class KalmanFilterZ_t{

private:
    real r;
    real q;

    real x_last;
    real p_last;

    real p_mid;
    real kg;

    bool init = false;

public:
    template<typename U>
    KalmanFilterZ_t(const U & _r,const U & _q ):r(static_cast<real>(_r)), q(static_cast<real>(_q)){;}

    template<typename U>
    const real update(const U x);

    template<typename U>
    const real predict(const U x);
};

#include "KalmanFilter.tpp"

typedef KalmanFilterZ_t<real_t> KalmanFilterZ;

#endif

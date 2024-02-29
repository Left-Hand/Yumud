#ifndef __KALMAN_FILTER_HPP__

#define __KALMAN_FILTER_HPP__

#include "real.hpp"

template<typename T>
class KalmanFilter_t{

private:
    T r;
    T q;

    T x_last;
    T p_last;

    T p_mid;
    T kg;

    bool init = false;

public:
    template<typename U>
    KalmanFilter_t(const U & _r,const U & _q ):r(static_cast<T>(_r)), q(static_cast<T>(_q)){;}

    template<typename U>
    const T update(const U & x);

    template<typename U>
    const T predict(const U & x);
};

#include "KalmanFilter.tpp"

typedef KalmanFilter_t<real_t> KalmanFilter;

#endif

#ifndef __KALMANFILTER_HPP__

#define __KALMANFILTER_HPP__

#include "../types/real.hpp"

template<typename T>
class KalmanFilter {

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
    KalmanFilter(T _r = T(1), T _q = T(1)):r(static_cast<T>(_r)), q(static_cast<T>(_q)){;}
    
    template<typename U>
    const T update(const U & x);

    template<typename U>
    const T predict(const U & x);
};

#endif

#pragma once

#include "sys/math/real.hpp"

template<typename T>
requires std::is_arithmetic_v<T>
class KalmanFilterZ_t{

private:
    T r;
    T q;

    T x_last;
    T p_last;

    T p_mid;
    T kg;

    bool init = false;

public:
    KalmanFilterZ_t(const arithmetic auto & _r,const arithmetic auto & _q ):r(static_cast<T>(_r)), q(static_cast<T>(_q)){;}

    const T update(const arithmetic auto x);

    const T predict(const arithmetic auto x);
};

#include "KalmanFilter.tpp"

typedef KalmanFilterZ_t<real_t> KalmanFilterZ;


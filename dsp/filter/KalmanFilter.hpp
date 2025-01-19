#pragma once

#include "sys/core/platform.h"

template<arithmetic T>
class KalmanFilter_t{

private:
    T r;
    T q;

    T x_last;
    T p_last;

    T p_mid;
    T kg;

    bool inited = false;
public:
    KalmanFilter_t(
        const T _r,
        const T _q
    ):
        r(static_cast<T>(_r)), 
        q(static_cast<T>(_q)){;}

    const T update(const arithmetic auto x);

    const T predict(const arithmetic auto x);

    void reset(){
        p_last = 0;
        inited = false;
    }
};

#include "KalmanFilter.tpp"


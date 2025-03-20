#pragma once

#include "core/platform.h"

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

template<arithmetic T>
const T KalmanFilter_t<T>::update(const arithmetic auto _x) {
    T x = static_cast<T>(_x);
    if (!inited) {
        x_last = x;
        p_last = T(0);
        inited = true;
    }else {
        x_last = predict(x);
        p_last = (1-kg)*p_mid;
    }
    
    return x_last;
}

template<arithmetic T>
const T KalmanFilter_t<T>::predict(const arithmetic auto _x) {
    T x = static_cast<T>(_x);
    p_mid = p_last + q;
    kg = p_mid / (p_mid + r);
    return (x_last + kg * (x - x_last));
}

#ifndef __PID_H__

#define __PID_H__

#include "real.hpp"
#include <algorithm>

template<typename T>
class PID_t{
    protected:
        T kp = T(0.0);
        T ki = T(0.0);
        T kd = T(0.0);


        T _integral = T(0.0);
        T err = T(0.0);
        T err_last = T(0.0);

        T clp_min = T(-1.0f);
        T clp_max = T(1.0f);


    public:
        PID_t() = delete;

        template<typename U>
        PID_t(const U & _kp,const U & _ki = U(0),const U & _kd = U(0), const U & _cl = U(1)):
                kp(static_cast<T>(_kp)), ki(static_cast<T> (_ki)), kd(static_cast<T>(_kd)){
                    setClamp(_cl);
                }

        template<typename U>
        T update(const U & setpoint, const U & pv);

        template<typename U>
        void setClamp(const U & _clp){
            clp_min = static_cast<T>(-abs(_clp)),
            clp_max = static_cast<T>(abs(_clp));}

        template<typename U>
        void setClamp(const U & _clp_min, const U & _clp_max){
            clp_min = static_cast<T>(_clp_min),
            clp_max = static_cast<T>(_clp_max);}
};

#include "pid.tpp"

typedef PID_t<real_t> PID;
#endif
#ifndef __PID_H__

#define __PID_H__

#include "../types/real.hpp"
#include <algorithm>

template<typename T>
class PID{
    protected:
        T kp = T(0.0);
        T ki = T(0.0);
        T kd = T(0.0);


        T _integral = T(0.0);
        T err = T(0.0);       
        T err_last = T(0.0);

        
        T clp_min = T(-514.0f);
        T clp_max = T(514.0f);
    public:
        PID() = delete;

        template<typename U>
        PID(const U & _kp,const U & _ki,const U & _kd = T(0), const U & dur = T(1)):
                kp(static_cast<T>(_kp)), ki(static_cast<T> (_ki * dur)), kd(static_cast<T>(_kd / dur)){;}

        template<typename U>
        T update(const U & setpoint, const U & pv);

        template<typename U>
        void setClamp(const U & _clp){
            clp_min = static_cast<T>(abs(_clp)), 
            clp_max = static_cast<T>(-abs(_clp));}

        template<typename U>
        void setClamp(const U & _clp_min, const U & _clp_max){
            clp_min = static_cast<T>(_clp_min), 
            clp_max = static_cast<T>(_clp_max);}
};

#endif
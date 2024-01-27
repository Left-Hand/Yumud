#ifndef __PID_H__

#define __PID_H__

#include "../types/real.hpp"
#include <algorithm>

class PID{
    protected:
        real_t kp = real_t(0.0);
        real_t ki = real_t(0.0);
        real_t kd = real_t(0.0);


        real_t _integral = real_t(0.0);
        real_t err = real_t(0.0);       
        real_t err_last = real_t(0.0);

        
        real_t clp_min = real_t(-514.0f);
        real_t clp_max = real_t(514.0f);
    public:
        PID() = delete;
        PID(const real_t & _kp,const real_t & _ki,const real_t & _kd, const real_t & dur = real_t(1.0f)):kp(_kp), ki(_ki * dur), kd(_kd / dur){;}
        real_t update(const real_t & setpoint, const real_t & pv);

        void setClamp(const real_t & _clp){clp_min = real_t(0.0f) - _clp; clp_max = _clp;}
        void setClamp(const real_t & _clp_min, const real_t & _clp_max){clp_min = _clp_min, clp_max = _clp_max;}
};

#endif
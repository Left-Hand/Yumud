#ifndef __TINY_FOC_HPP__

#define __TINY_FOC_HPP__

#include "svpwm.hpp"
#include "device/Encoder/Odometer.hpp"
#include "dsp/controller/pid.hpp"

class MotorWithFoc:public ServoCloseLoop{
protected:
    SVPWM & svpwm;
    Odometer & odo;
    PID_t<real_t> & pos_pid;

public:
    MotorWithFoc(SVPWM & _svpwm, Odometer & _odo, PID_t<real_t> & _pos_pid):svpwm(_svpwm), odo(_odo), pos_pid(_pos_pid){;}
    void setMaxCurrent(real_t _max_current){
        pos_pid.setClamp(_max_current);
    }

    void closeLoopPos(const real_t & targ){
        odo.update();
        real_t pos = odo.getPosition();

        real_t curr = pos_pid.update(targ, pos);
        svpwm.setDQCurrent(real_t(0), curr, odo.getElecRad());
    }

    real_t getPosition() override{
        return odo.getPosition();
    }
};


#endif
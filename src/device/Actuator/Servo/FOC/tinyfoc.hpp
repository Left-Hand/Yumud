#ifndef __TINY_FOC_HPP__

#define __TINY_FOC_HPP__

#include "src/robots/stepper/svpwm.hpp"
#include "device/Encoder/Odometer.hpp"
#include "dsp/controller/pid.hpp"

class MotorWithFoc:public ServoCloseLoop{
protected:
    SVPWM & svpwm;
    OdometerPoles & odo;
    PID_t<real_t> & pos_pid;

    union{
        real_t target_pos;
        real_t target_vel;
        real_t target_curr;
    };

    enum class CtrlMethod:uint8_t{
        OPEN,
        LAPPOS,
        POS,
        VEL,
        CURR
    }
    ctrl_method = CtrlMethod::OPEN;
public:
    void init() override{;}
    void enable(const bool & en = true) override{
        svpwm.enable(en);
        enabled = en;
    }
    MotorWithFoc(SVPWM & _svpwm, OdometerPoles & _odo, PID_t<real_t> & _pos_pid):svpwm(_svpwm), odo(_odo), pos_pid(_pos_pid){;}
    void setMaxCurrent(real_t _max_current){
        // pos_pid.setClamp(_max_current);
        svpwm.setClamp(_max_current);
    }

    void run(){
        if(!enabled) return;
        odo.update();
        switch(ctrl_method){
        case CtrlMethod::OPEN:
            break;
        case CtrlMethod::LAPPOS:
            {
                real_t real_pos = odo.getPosition();
                real_t curr = pos_pid.update(round(real_pos - target_pos) + target_pos, real_pos);
                svpwm.setDQCurrent(real_t(0), curr, odo.getElecRad());
            }
            break;
        case CtrlMethod::POS:
            {
                real_t real_pos = odo.getPosition();
                real_t curr = pos_pid.update(target_pos, real_pos);
                svpwm.setDQCurrent(real_t(0), curr, odo.getElecRad());
            }
            break;
        case CtrlMethod::VEL:
            break;
        case CtrlMethod::CURR:
            svpwm.setDQCurrent(real_t(0), target_curr, odo.getElecRad());
            break;
        }
    }

    void trackPos(const real_t & targ){
        target_pos = targ;
        ctrl_method = CtrlMethod::POS;
    }

    void trackVel(const real_t & targ){
        target_vel = targ;
        ctrl_method = CtrlMethod::VEL;
    }

    void trackCurr(const real_t & targ){
        target_curr = targ;
        ctrl_method = CtrlMethod::CURR;
    }

    void trackLapPos(const real_t & targ){
        target_pos = targ;
        ctrl_method = CtrlMethod::LAPPOS;
    }

    void open(){
        ctrl_method = CtrlMethod::OPEN;
    }

    real_t getPosition() override{
        return odo.getPosition();
    }
};


#endif
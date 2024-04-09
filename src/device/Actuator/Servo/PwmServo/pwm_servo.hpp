#ifndef __PWM_SERVO_HPP__
#define __PWM_SERVO_HPP__

#include "src/device/Actuator/Actuator.hpp"


class PwmServo:public ServoOpenLoop{
protected:
    PwmChannelConcept & instance;
    const real_t min_value_duty;
    const real_t max_value_duty;
    bool enabled = true;
public:
    PwmServo(PwmChannelConcept & _instance, const real_t & _min_value_duty, const real_t & _max_value_duty
            ):instance(_instance), min_value_duty(_min_value_duty), max_value_duty(_max_value_duty){;}

    void init() override {
        instance.init();
    }

    void enable(const bool & en = true) override {
        enabled = en;
        if(!en) setDuty(real_t(0));
    }

    virtual void setDuty(const real_t & duty) = 0;
};


class PwmAngleServo:public PwmServo{
protected:
    Range_t<real_t> angle_elec_range;
    Range_t<real_t> angle_phy_range;


public:
    PwmAngleServo(PwmChannelConcept & _instance, const int & _angle_scale, const real_t & _min_value_duty = real_t(0.025), const real_t & _max_value_duty = real_t(0.125)):
            PwmServo(_instance, _min_value_duty, _max_value_duty), angle_elec_range(real_t(_angle_scale < 0  ?  - _angle_scale:0),real_t( _angle_scale < 0 ? 0 : _angle_scale)){;}

    void setDuty(const real_t & duty) override{
        if(!enabled) return instance.setDuty(real_t(0));
        instance.setDuty(LERP(CLAMP(duty, real_t(0), real_t(1)), min_value_duty, max_value_duty));
    }

    void setAngle(const real_t & angle){
        setDuty(CLAMP(angle_elec_range.invlerp(angle), real_t(0), real_t(1)));
    }
};

class PwmSpeedServo:public PwmServo{
protected:
    real_t max_rot_per_second;

    PwmSpeedServo(PwmChannelConcept & _instance, const real_t & _min_value_duty, const real_t & _max_value_duty, const int & _max_rot_per_second
            ):PwmServo(_instance, _min_value_duty, _max_value_duty), max_rot_per_second(_max_rot_per_second){;}
public:
    void setDuty(const real_t & duty) override{
        if(!enabled) return instance.setDuty(real_t(0));
        instance.setDuty(LERP(CLAMP(duty, real_t(-1), real_t(1)) * real_t(0.5) + real_t(0.5), min_value_duty, max_value_duty));
    }

    void setSpeed(const real_t & rps){
        setDuty(rps / max_rot_per_second);
    }
};

class Servo180: public PwmAngleServo{
public:
    Servo180(PwmChannelConcept & _instance):PwmAngleServo(_instance, 180){;}
};


class Servo270: public PwmAngleServo{
public:
    Servo270(PwmChannelConcept & _instance):PwmAngleServo(_instance, 270){;}
};


class Servo360: public PwmSpeedServo{
public:
    Servo360(PwmChannelConcept & _instance):PwmSpeedServo(_instance, real_t(0.025), real_t(0.125), 270){;}
};


#endif

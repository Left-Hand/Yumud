#pragma once

#include "drivers/Actuator/servo/servo.hpp"
#include "types/range/range_t.hpp"

class ScaledPwm:public PwmChannel{
protected:
    PwmChannel & instance_;
    Range duty_range_;
    bool enabled = true;
public:
    ScaledPwm(PwmChannel & _instance, const Range & _duty_range
            ):instance_(_instance), duty_range_(_duty_range){;}

    void enable(const bool en = true){
        enabled = en;
        if(false == en) *this = 0;
    }

    ScaledPwm & operator = (const real_t duty) override {
        if(false == enabled) return *this;
        instance_ = duty_range_.lerp(duty);
        return *this;
    }
};


class PwmRadianServo:public RadianServo{
protected:
    ScaledPwm instance_;
    Range_t<real_t> angle_elec_range;

    void setGLobalRadian(const real_t rad) override{
        
    }

    real_t getGlobalRadian() override{
        return 0;
    }
    PwmRadianServo(PwmChannel & _instance, 
        const int _angle_scale, 
        const Range & _duty_range = {real_t(0.025), real_t(0.125)}
        ):
        
            instance_(_instance, _duty_range), 
            angle_elec_range(real_t(_angle_scale < 0  ?  - _angle_scale:0),real_t( _angle_scale < 0 ? 0 : _angle_scale))
            
            {;}
};

// class PwmSpeedServo:public PwmServo{
// protected:
//     real_t max_rot_per_second;

//     PwmSpeedServo(PwmChannel & _instance, const real_t & _min_value_duty, const real_t & _max_value_duty, const int & _max_rot_per_second
//             ):PwmServo(_instance, _min_value_duty, _max_value_duty), max_rot_per_second(_max_rot_per_second){;}
// public:
//     void setDuty(const real_t duty) override{
//         if(!enabled){(instance = (real_t(0))); return;}
//         instance = (LERP(CLAMP(duty, real_t(-1), real_t(1)) * real_t(0.5) + real_t(0.5), min_value_duty, max_value_duty));
//     }

//     void setSpeed(const real_t rps){
//         setDuty(rps / max_rot_per_second);
//     }
// };





class Servo180: public PwmRadianServo{
public:
    Servo180(PwmChannel & _instance):PwmRadianServo(_instance, 180){;}
};


class Servo270: public PwmRadianServo{
public:
    Servo270(PwmChannel & _instance):PwmRadianServo(_instance, 270){;}
};


// class Servo360: public PwmSpeedServo{
// public:
//     Servo360(PwmChannel & _instance):PwmSpeedServo(_instance, real_t(0.025), real_t(0.125), 270){;}
// };


#pragma once

#include "drivers/Actuator/servo/servo.hpp"
#include "types/range/range_t.hpp"

namespace ymd::drivers{

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
        if(false == enabled){
            instance_ = 0;
            return *this;
        }else{
            instance_ = duty_range_.lerp(duty);
            return *this;
        }
    }

    auto & inst(){
        return instance_;
    }
};


class PwmRadianServo:public RadianServo{
protected:
    ScaledPwm instance_;
    real_t last_rad;

    void setGLobalRadian(const real_t rad) override{
        instance_ = (rad) * real_t(1 / PI);
        last_rad = rad;
    }

    real_t getGlobalRadian() override{
        return last_rad;
    }
    
public:
    PwmRadianServo(PwmChannel & instance):
            instance_(instance, {real_t(0.025), real_t(0.125)})
            {;}

    void idle() override{
        instance_.enable(false);
    }

};

class PwmSpeedServo:public SpeedServo{
protected:
    ScaledPwm instance_;
    real_t max_turns_per_second_;
    real_t expect_speed_;

    void setSpeedDirectly(const real_t rps) override{
        expect_speed_ = rps;
        setDuty(rps / max_turns_per_second_);
    }

    real_t getSpeed() override{
        return expect_speed_;
    }

    void setDuty(const real_t duty){
        instance_ = (duty + 1) * real_t(0.5);
    }
public:
    PwmSpeedServo(PwmChannel & instance, const real_t max_turns_per_second = 2):
            instance_(instance, {real_t(0.025), real_t(0.125)}),
            max_turns_per_second_(max_turns_per_second),
            expect_speed_(real_t(0))
            {;}


};





// class Servo180: public PwmRadianServo{
// public:
//     Servo180(PwmChannel & _instance):PwmRadianServo(_instance, 180){;}
// };


// class Servo270: public PwmRadianServo{
// public:
//     Servo270(PwmChannel & _instance):PwmRadianServo(_instance, 270){;}
// };


// class Servo360: public PwmSpeedServo{
// public:
//     Servo360(PwmChannel & _instance):PwmSpeedServo(_instance, real_t(0.025), real_t(0.125), 270){;}
// };

    using MG995 = PwmRadianServo;
    using SG90 = PwmRadianServo;
}
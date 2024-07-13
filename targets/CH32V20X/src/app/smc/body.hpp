#ifndef __BODY_HPP__

#define __BODY_HPP__

#include "fans.hpp"

namespace SMC{

struct MotorStrength{
    real_t left =   0;
    real_t right =  0;
    real_t hri = 0;
    real_t chassis = 0;
};


class RigidBody{
protected:
    MotorStrength & motor_strength;

    SideFan & left_fan;
    SideFan & right_fan;
    HriFanPair & hri_fan;
    ChassisFanPair & chassis_fan;

public:
    RigidBody(MotorStrength & _motor_strength, SideFan & _left_fan, SideFan & _right_fan, HriFanPair & _hri_fan, ChassisFanPair & _chassis_fan):
            motor_strength(_motor_strength), left_fan(_left_fan), right_fan(_right_fan), hri_fan(_hri_fan), chassis_fan(_chassis_fan){;}

    void enable(const bool & en = true){
        left_fan.enable(en);
        right_fan.enable(en);
        hri_fan.enable(en);
        chassis_fan.enable(en);
    }

    void init(){
        left_fan.init();
        right_fan.init();
        hri_fan.init();
        chassis_fan.init();
    }

    void update(){
        if(motor_strength.chassis > 0.05){
            left_fan = motor_strength.left;
            right_fan = motor_strength.right;
            hri_fan = motor_strength.hri;
        }else{
            left_fan = real_t(0);
            right_fan = real_t(0);
            hri_fan  = real_t(0);
        }

        chassis_fan = motor_strength.chassis;
    }
};

};


#endif
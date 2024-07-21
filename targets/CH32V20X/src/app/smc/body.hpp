#ifndef __BODY_HPP__

#define __BODY_HPP__

#include "fans.hpp"

namespace SMC{

struct MotorStrength{
    real_t left;
    real_t right;
    real_t hri ;
    real_t chassis ;

    MotorStrength(){reset();}
    void reset(){
        left = 0;
        right = 0;
        hri = 0;
        chassis = 0;
    }
};


class RigidBody{
protected:
    MotorStrength & motor_strength;

    SideFan & left_fan;
    SideFan & right_fan;
    HriFanPair & hri_fan;

    real_t real_cha_output = 0;

    bool enabled = false;

    void setds(const real_t ds){
        real_cha_output = ds;
        static DShotChannel ch1{timer8.oc(1)};
        static DShotChannel ch2{timer8.oc(2)};

        ch1.init();
        ch2.init();
        ch1 = ds;
        ch2 = ds;
    }
public:
    RigidBody(MotorStrength & _motor_strength, SideFan & _left_fan, SideFan & _right_fan, HriFanPair & _hri_fan):
            motor_strength(_motor_strength), left_fan(_left_fan), right_fan(_right_fan), hri_fan(_hri_fan){;}

    void enable(const bool en = true){
        enabled = en;
        left_fan.enable(en);
        right_fan.enable(en);
        hri_fan.enable(en);
        if(enabled == false){
            setds(0);
        }
    }

    void init(){
        left_fan.init();
        right_fan.init();
        hri_fan.init();

        left_fan.enable();
        right_fan.enable();
        hri_fan.enable();
    }

    void reset(){
        motor_strength.reset();
        left_fan.reset();
        right_fan.reset();
        hri_fan.reset();
        setds(0);
    }

    void update(){
        real_cha_output = MIN(real_cha_output + 0.003, motor_strength.chassis);

        if(real_cha_output > 0.4 and enabled){
            left_fan = motor_strength.left;
            right_fan = motor_strength.right;
            hri_fan = motor_strength.hri;
        }else{
            left_fan = real_t(0);
            right_fan = real_t(0);
            hri_fan  = real_t(0);
        }

        setds(real_cha_output);
    }
};

};


#endif
#pragma once

#include "../../common/inc.hpp"
#include "../actuator/jointlr.hpp"
#include "claw.hpp"
#include "nozzle.hpp"

namespace gxm{

class Scara{
public:
    struct Config{
        Scara5Solver::Config scara_config;
        JointLR::Config joint_config;
    };

protected:

    struct Refs{
        std::reference_wrapper<JointLR> joint_l;
        std::reference_wrapper<JointLR> joint_r;
        std::reference_wrapper<Claw> claw;
        std::reference_wrapper<Nozzle> nozzle;
    };
        
    Config config_;
    Scara5Solver solver_{config_.scara_config};

    JointLR & joint_l;
    JointLR & joint_r;
    Claw & claw;
    Nozzle & nozzle;
public:
    void goHome();//进行坐标归位
    void moveXY(const Vector2 & pos);//只改变XY坐标

    void pickUp();//拾起物块
    void putDown();//放下物块

    Scara(const Config & config, const Refs & refs):
        config_(config),
        joint_l(refs.joint_l),
        joint_r(refs.joint_r),
        claw(refs.claw),
        nozzle(refs.nozzle){}
};
    
}
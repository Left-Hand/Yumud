#pragma once

#include "../actuator/jointlr.hpp"
#include "robots/kinematics/Scara5/scara5_solver.hpp"
#include "claw.hpp"
#include "nozzle.hpp"

namespace ymd::robots{

class Scara{
public:
    struct Config{
        Scara5Solver_t<real_t>::Config solver_config;
        // JointLR::Config joint_config;
        Claw::Config claw_config;
        Nozzle::Config nozzle_config;
    };

protected:

    struct Refs{
        std::reference_wrapper<JointLR> joint_l;
        std::reference_wrapper<JointLR> joint_r;
        std::reference_wrapper<Claw> claw;
        std::reference_wrapper<Nozzle> nozzle;
    };
        
    Config config_;
    Scara5Solver_t<real_t> solver_{config_.solver_config};

    JointLR & joint_l;
    JointLR & joint_r;
    Claw & claw;
    Nozzle & nozzle;
public:
    void goHome();//进行坐标归位
    void moveXY(const Vector2q<16> & pos);//只改变XY坐标
    Vector2q<16> getPos();
    void press();//拾起物块
    void release();//放下物块
    void idle();
    bool reached();
    bool caught();

    Scara(const Config & config, const Refs & refs):
        config_(config),
        joint_l(refs.joint_l),
        joint_r(refs.joint_r),
        claw(refs.claw),
        nozzle(refs.nozzle){}

    DELETE_COPY_AND_MOVE(Scara)
};
    
}
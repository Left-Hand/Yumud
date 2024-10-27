#pragma once

#include "motion_module.hpp"

namespace gxm{

class GrabModule:public MotionModule{
public:
    struct Config
            // public Scara5Solver::Config, 
            // public ZAxis::Config,
            // public JointLR::Config,
            // public Claw::Config,
            // public Nozzle::Config
    {

        Scara5Solver::Config scara_config;
        ZAxis::Config zaxis_config;
        JointLR::Config joint_config;
        Claw::Config claw_config;
        Nozzle::Config nozzle_config;


    };

protected:


    struct Refs{
        std::reference_wrapper<ZAxis> zaxis;
        std::reference_wrapper<JointLR> joint_l;
        std::reference_wrapper<JointLR> joint_r;
        std::reference_wrapper<Claw> claw;
        std::reference_wrapper<Nozzle> nozzle;
    };
        
    Config config_;
    Scara5Solver solver_{config_.scara_config};

    ZAxis & zaxis;
    JointLR & joint_l;
    JointLR & joint_r;
    Claw & claw;
    Nozzle & nozzle;
public:
    void goHome();//进行坐标归位
    void moveZ(const real_t pos);//只改变Z轴坐标
    void moveXY(const Vector2 & pos);//只改变XY坐标
    void moveTo(const Vector3 & pos);//改变所有坐标

    void pickUp();//拾起物块
    void putDown();//放下物块

    // Vector2 calculatePos(TrayIndex index)
public:
    GrabModule(const Config & config, const Refs & refs):
        config_(config),
        zaxis(refs.zaxis),
        joint_l(refs.joint_l),
        joint_r(refs.joint_r),
        claw(refs.claw),
        nozzle(refs.nozzle){}

    void take();
    void give();
    bool done();
    void begin();
};
    
}
#pragma once

#include "motion_module.hpp"

#include "actuator/jointlr.hpp"
#include "actuator/zaxis.hpp"
#include "scara/claw.hpp"
#include "scara/nozzle.hpp"
#include "scara/scara.hpp"


namespace gxm{

class GrabModule:public MotionModule{
public:
    struct Config{

        Scara::Config scara_config;
        ZAxis::Config zaxis_config;
    };

protected:


    struct Refs{
        std::reference_wrapper<ZAxis> zaxis;
        std::reference_wrapper<Scara> scara;
    };
        
    Config config_;
    ZAxis & zaxis;
    Scara & scara;
    
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
        scara(refs.scara)
        {}

    void take();
    void give();
    bool done();
    void begin();
};
    
}
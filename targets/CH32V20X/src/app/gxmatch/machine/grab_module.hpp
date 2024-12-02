#pragma once

#include "motion_module.hpp"

#include "actuator/jointlr.hpp"
#include "actuator/zaxis.hpp"
#include "scara/claw.hpp"
#include "scara/nozzle.hpp"
#include "scara/scara.hpp"


namespace gxm{

class GrabModule;

namespace GrabActions{

class GrabAction:public Action{
protected:
    using Inst = GrabModule; 
    Inst & inst_;
    GrabAction(size_t s, Callback && func, Inst & inst):
        Action(s, std::move(func)),
        inst_(inst){;}
public:
};


}

class GrabModule:public MotionModule{
public:
    struct Config{
        std::array<Vector2, 3> tray_xy;
        real_t tray_z;
        
        Vector2 inspect_xy;

        AABB blocked_area;
        real_t max_spd;
        real_t max_acc;
    };

protected:


    struct Refs{
        std::reference_wrapper<ZAxis> zaxis;
        std::reference_wrapper<Scara> scara;
    };
        
    Config config_;
    ZAxis & zaxis_;
    Scara & scara_;

public:
// protected:
    
public:
// protected:
    void goHome();//进行坐标归位
    void moveZ(const real_t pos);//只改变Z轴坐标
    void moveXY(const Vector2 & pos);//只改变XY坐标
    void moveTo(const Vector3 & pos);//改变所有坐标
    Vector3 getPos();

    void press();//拾起物块
    void release();//放下物块

    Vector3 calculateTrayPos(TrayIndex index);
public:
    GrabModule(const Config & config, const Refs & refs):
        config_(config),
        zaxis_(refs.zaxis),
        scara_(refs.scara)
        {}

    void take();
    void give();
    void begin();
    
    bool reached();

    const auto & config(){return config_;}
};

struct GrabSysConfig{
    Scara::Config scara_config;
    ZAxis::Config zaxis_config;
    GrabModule::Config grab_config;
    CrossSolver::Config cross_config;

};




}
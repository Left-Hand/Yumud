#pragma once

#include "motion_module.hpp"

#include "actuator/jointlr.hpp"
#include "actuator/zaxis_cross.hpp"
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
        real_t free_z;
        real_t catch_z;
        real_t z_bias;
        
        Vector2 catch_xy;
        Vector3 inspect_xyz;

        Vector3 home_xyz;

        AABB safe_aabb;
        real_t max_spd;
        real_t max_acc;

        uint nozzle_sustain;
    };

    enum class TranportStatus{
        AIR,
        OUTER,
        INNER
    };

    TranportStatus status_;


protected:


    struct Refs{
        std::reference_wrapper<ZAxisCross> zaxis;
        std::reference_wrapper<Scara> scara;
    };
        
    Config config_;
    ZAxisCross & zaxis_;
    Scara & scara_;



public:
// protected:
    
public:
// protected:
    void goHome();//进行坐标归位
    // void moveZ(const real_t pos);//只改变Z轴坐标
    // void moveXY(const Vector2 & pos);//只改变XY坐标
    void rapid(const Vector3 & pos);//改变所有坐标
    Vector3 getPos();
    void air_take_air();
    void air_give_air(const TrayIndex tray_index);
    void air_inspect();
    void take_place(const TrayIndex tray_index);
    void give_place();
    void to_air();
    void press();
    void release();

    Vector2 calculateTrayPos(const TrayIndex index);
public:
    GrabModule(const Config & config, const Refs & refs):
        config_(config),
        zaxis_(refs.zaxis),
        scara_(refs.scara)
        {}

    void init();
    void move(const Vector3 & pos);
    void inspect();
    void take(const TrayIndex index);
    void give(const TrayIndex index);
    void test();
    
    bool reached();

    const auto & config(){return config_;}
    void setStatus(const TranportStatus status){
        status_ = status;
    }
};



}
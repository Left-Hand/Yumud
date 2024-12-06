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
        
        std::array<Vector2, 3> catch_xy;
        Vector3 inspect_xyz;
        Vector3 idle_xyz;

        Vector3 home_xyz;

        AABB safe_aabb;
        real_t max_spd;
        real_t max_acc;

        real_t max_z_spd;
        real_t max_z_acc;

        uint nozzle_sustain;
    };


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
    void meta_air_take_air(const TrayIndex tray_index);
    void meta_air_give_air(const TrayIndex tray_index);
    void meta_air_inspect();
    void meta_take_place(const TrayIndex tray_index);
    void meta_give_place(const TrayIndex tray_index);
    void meta_to_air();
    void meta_press();
    void meta_release();

    Vector2 calculateTrayPos(const TrayIndex index) const;
    Vector2 calculateCatchPos(const TrayIndex index) const;
public:
    GrabModule(const Config & config, const Refs & refs):
        config_(config),
        zaxis_(refs.zaxis),
        scara_(refs.scara)
        {}

    void init();
    void move(const Vector3 & pos);
    void move_xy(const Vector2 & pos);
    void move_z(const real_t z);
    void inspect();
    void idle();
    void take(const TrayIndex index);
    void give(const TrayIndex index);
    void test();
    
    bool reached();
    bool caught();

    void press();
    void release();

    const auto & config(){return config_;}
};



}
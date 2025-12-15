#pragma once

#include "motion_module.hpp"

#include "actuator/jointlr.hpp"
#include "actuator/zaxis_cross.hpp"
#include "scara/claw.hpp"
#include "scara/nozzle.hpp"
#include "scara/scara.hpp"

#include "algebra/vectors/vec3.hpp"
#include "algebra/regions/aabb.hpp"


namespace ymd::robots{

class GrabModule;

namespace GrabActions{

class GrabAction:public ActionBase{
protected:
    using Inst = GrabModule; 
    Inst & inst_;
    GrabAction(size_t s, Callback && func, Inst & inst):
        ActionBase(s, std::move(func)),
        inst_(inst){;}
public:
};


}

class GrabModule:public MotionModule{
public:
    struct Config{
        std::array<Vec2<iq16>, 3> tray_xy;
        real_t tray_z;
        real_t free_z;
        real_t catch_z;
        real_t z_bias;
        
        std::array<Vec2<iq16>, 3> catch_xy;
        Vec3<iq16> inspect_xyz;
        Vec3<iq16> idle_xyz;

        Vec3<iq16> home_xyz;

        AABB<real_t> safe_aabb;
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
    // void moveXY(const Vec2<iq16> & pos);//只改变XY坐标
    void rapid(const Vec3<iq16> & pos);//改变所有坐标
    Vec3<iq16> getPos();
    void meta_air_inspect();
    void meta_to_air();
    void meta_press();
    void meta_release();
    void meta_idle();

public:
    GrabModule(const Config & config, const Refs & refs):
        config_(config),
        zaxis_(refs.zaxis),
        scara_(refs.scara)
        {}

    void init();
    void move(const Vec3<iq16> & pos);
    void move_xy(const Vec2<iq16> & pos);
    void move_z(const real_t z);
    void inspect();
    void idle();
    void test();
    
    bool reached();
    bool caught();

    void press();
    void release();

    const auto & config(){return config_;}
};



}
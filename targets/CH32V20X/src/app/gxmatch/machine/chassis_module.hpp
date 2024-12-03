#pragma once

#include "motion_module.hpp"
#include "../autodrive/Estimator.hpp"
#include "wheel/wheels.hpp"



namespace gxm{

class ChassisModule;

namespace ChassisActions{

class ChassisAction:public Action{
protected:
    using Inst = ChassisModule; 
    Inst & inst_;
    ChassisAction(size_t s, Callback && func, Inst & inst):
        Action(s, std::move(func)),
        inst_(inst){;}
public:
};


}


class ChassisModule:public MotionModule{
public:
    class RotationCtrl{
    public:
        struct Config{
            real_t kp;
            real_t kd;  
        };

    protected:
        const Config & config_;
    public:
        RotationCtrl(const Config & config):config_(config){;}
        DELETE_COPY_AND_MOVE(RotationCtrl)
    };

    class PositionCtrl{
    public:
        struct Config{
            real_t kp;
            real_t kd;
        };

    protected:
        const Config & config_;
    public:
        PositionCtrl(const Config & config):config_(config){;}
        DELETE_COPY_AND_MOVE(PositionCtrl)
    };

    struct Config{
        Mecanum4Solver::Config solver_config;
        Wheel::Config wheel_config;

        real_t max_acc;
        real_t max_spd;
        
        real_t max_agr;
        real_t max_spr;
    };
    

    const Config & config_;
    Wheels wheels_;

    Mecanum4Solver solver_{config_.solver_config};
    
protected:

public:
    ChassisModule(const Config & config, const Wheels & wheels):config_(config), wheels_(wheels) {}

    void rapid(const Ray & ray);
    void rapid_move(const Vector2 & pos);
    void rapid_spin(const real_t & rad);

    void positionTrim(const Vector2 & trim);
    void rotationTrim(const real_t raderr);
    void forwardMove(const Vector2 & vel, const real_t spinrate);

    void calibratePosition(const Vector2 & pos);
    void calibrateRotation(const real_t rad);

    void test();
    bool arrived();

    Ray feedback();
    const auto & config()const {return config_;}
};

}
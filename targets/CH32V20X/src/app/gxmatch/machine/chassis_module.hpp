#pragma once

#include "motion_module.hpp"
#include "wheel/wheels.hpp"

namespace gxm{
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
    };
    

    const Config & config_;
    Wheels wheels_;

    Mecanum4Solver solver_{config_.solver_config};
    
public:
    ChassisModule(const Config & config, const Wheels & wheels):config_(config), wheels_(wheels) {}

    void positionTrim(const Vector2 & trim);
    void rotationTrim(const real_t raderr);
    void forwardMove(const Vector2 & vel, const real_t spinrate);

    void calibratePosition(const Vector2 & pos);
    void calibrateRotation(const real_t rad);

    void tick();

    const auto & config()const {return config_;}
};

}
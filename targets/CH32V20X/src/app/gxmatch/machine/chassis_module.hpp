#pragma once

#include "motion_module.hpp"


namespace gxm{
class ChassisModule:public MotionModule{
public:
    struct Config:public Mecanum4Solver::Config{
        
    };
    
    const Config & config_;

    Mecanum4Solver solver_{config_};
    
public:
    ChassisModule(const Config & _config):config_(_config){}

    void positionTrim(const Vector2 & trim);
    void rotationTrim(const real_t raderr);
    void forwardMove(const Vector2 & pos, const real_t rad);

    void calibratePosition(const Vector2 & pos);
    void calibrateRotation(const real_t rad);

    void tick();
};

}
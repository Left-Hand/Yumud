#pragma once

#include "motion_module.hpp"


namespace gxm{
class ChassisModule:public MotionModule{
public:
    struct Config:
            public Mecanum4Solver::Config,
            public Wheel::Config
            
    {
        
    };
    

    using Wheels = std::array<std::reference_wrapper<Wheel>, 4>;
    const Config & config_;
    Wheels wheels_;

    Mecanum4Solver solver_{config_};
    
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
#pragma once

#include "zaxis.hpp"
#include "jointlr.hpp"
#include "robots/kinematics/Cross/CrossSolver_t.hpp"


namespace ymd::drivers{
class RadianServo;
};

namespace ymd::robots{
class ZAxisCross:public ZAxis{
public:
    using Motor = JointLR;
    // using Motor = SG90;
    using Solver = CrossSolver_t<real_t>;

    struct Config:public ZAxis::Config{
        Solver::Config solver_config;    
    };
private:
    const Config & config_;
protected:
    Motor & motor_;
    Solver solver_;
public:
    ZAxisCross(const Config & config, Motor & motor):
        ZAxis(config), config_(config), motor_(motor), solver_{config.solver_config}{}
    DELETE_COPY_AND_MOVE(ZAxisCross)
    
    bool reached() override;
    void setDistance(const real_t dist) override;  
    real_t getDistance() override;

    void tick() override;
    void idle(){
        motor_.idle();
    }

    void softHome() override;
};


}
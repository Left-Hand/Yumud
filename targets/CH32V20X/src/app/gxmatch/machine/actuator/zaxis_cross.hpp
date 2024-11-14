#pragma once

#include "src/app/gxmatch/common/inc.hpp"
#include "zaxis.hpp"
#include "JointCl.hpp"


namespace ymd::drivers{
class RadianServo;
};

namespace gxm{
class ZAxisCross:public ZAxis{
public:
    using Motor = gxm::JointCl;
    using Solver = gxm::CrossSolver;

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

    void softHome() override;
};


}
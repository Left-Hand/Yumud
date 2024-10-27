#pragma once

#include "../../common/inc.hpp"

namespace gxm{

class Wheel{
public:
    struct Config{
        real_t wheel_radius = real_t(0.06);
        real_t max_tps = real_t(0.5);
        real_t max_aps = real_t(0.5);
    };
    
protected:
    const Config & config_;

    virtual void setMotorSpeed(const real_t spd) = 0;
    virtual void setMotorPosition(const real_t pos) = 0;
    virtual real_t getMotorSpeed() = 0;
    virtual real_t getMotorPosition() = 0;
public:
    Wheel(const Config & config):config_(config){;}

    void setSpeed(const real_t spd);
    void setPosition(const real_t pos);

    real_t getSpeed();
    real_t getPosition();
};


}
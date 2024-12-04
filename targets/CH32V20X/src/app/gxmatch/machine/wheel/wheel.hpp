#pragma once

#include "../../common/inc.hpp"
#include "robots/foc/focmotor.hpp"


namespace gxm{

class Wheel{
public:
    struct Config{
        real_t wheel_radius;
        real_t max_tps;
        real_t max_aps;
    };
    
protected:
    using Motor = ymd::foc::FOCMotorConcept;

    // const FocMotor & config_;
private:
    const Config & config_;
protected:
    Motor & motor_;


    __fast_inline constexpr real_t World2Motor(const real_t x){
        return x / (real_t(TAU) * config_.wheel_radius);
    }

    __fast_inline constexpr real_t Motor2World(const real_t x){
        return x * (real_t(TAU) * config_.wheel_radius);
    }
protected:
    virtual void setMotorSpeed(const real_t spd) = 0;
    virtual void setMotorPosition(const real_t pos) = 0;
    virtual real_t getMotorSpeed() = 0;
    virtual real_t getMotorPosition() = 0;

    real_t last_targ_position;
public:
    Wheel(const Config & config, Motor & motor):
        config_(config), motor_(motor){;}

    // void setSpeed(const real_t spd);
    void setPosition(const real_t pos);
    void forwardPosition(const real_t step);

    // real_t getSpeed();
    real_t getPosition();
};


}
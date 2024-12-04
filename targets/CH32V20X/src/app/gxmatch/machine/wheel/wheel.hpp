#pragma once

#include "../../common/inc.hpp"
#include "robots/foc/focmotor.hpp"
#include "robots/foc/remote/remote.hpp"

namespace gxm{

class Wheel{
public:
    struct Config{
        real_t wheel_radius;
        real_t max_tps;
        real_t max_aps;
    };
    
    using Motor = ymd::foc::FOCMotorConcept;
protected:
    bool inversed_ = false;
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
    void setMotorPosition(const real_t pos);
    real_t getMotorPosition();

    real_t last_targ_position;
public:
    Wheel(const Config & config, Motor & motor):
        config_(config), motor_(motor){;}

    // void setSpeed(const real_t spd);
    void setPosition(const real_t pos);
    void setCurrent(const real_t curr);
    void forwardPosition(const real_t step);

    void inverse(const bool en = true){
        inversed_ = en;
    }

    // real_t getSpeed();
    real_t getPosition();

    auto & motor() {return motor_;}
};


}
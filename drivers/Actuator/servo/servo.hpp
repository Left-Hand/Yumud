#pragma once

#include "drivers/Actuator/Actuator.hpp"


class RadianServo{
    virtual void setGLobalRadian(const real_t rad) = 0;
    virtual real_t getGlobalRadian() = 0;
public:
    void setRadian(const real_t rad){
        setGLobalRadian(rad);    
    }

    real_t getRadian(){
        return getGlobalRadian();
    }
};


class SpeedServo{
    virtual void setSpeedDirectly(const real_t rad) = 0;
public:
    void setSpeed(const real_t rad){
        setSpeedDirectly(rad);    
    }

    virtual real_t getSpeed() = 0;
};
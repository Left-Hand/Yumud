#pragma once

#include "drivers/Actuator/Actuator.hpp"
#include "types/range/range_t.hpp"

namespace ymd::drivers{

class RadianServo{
private:
    Range rad_range_ = {0, real_t(PI)};
    real_t basis_ = 0;
protected:
    
    virtual void setGLobalRadian(const real_t rad) = 0;
    virtual real_t getGlobalRadian() = 0;
public:
    void setRange(const Range & range){
        rad_range_ = range;
    }
    
    void setBasis(const real_t rad){
        basis_ = rad;
    }
    
    void setRadian(const real_t rad){
        setGLobalRadian(rad_range_.clamp(rad + basis_));
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

};
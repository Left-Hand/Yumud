#pragma once

#include "drivers/Actuator/Actuator.hpp"
#include "core/math/real.hpp"

namespace ymd::drivers{

class RadianServo{
private:
    Range2_t<real_t> rad_range_ = {0, real_t(PI)};
    // real_t basis_ = 0;
protected:
    
    virtual void set_global_radian(const real_t rad) = 0;
    virtual real_t get_global_radian() = 0;
public:
    void set_range(const Range2_t<real_t> & range){
        rad_range_ = range;
    }
    
    void set_radian(const real_t rad){
        // setGLobalRadian(rad_range_.clamp(rad + basis_));
        set_global_radian(rad_range_.clamp(rad));
        // if(rad_range_.has(rad) == false){
            // DEBUG_PRINTLN());
        //     HALT;
        // }


        // setGLobalRadian(rad);
    }

    void idle();

    real_t get_radian(){
        return get_global_radian();
    }

};


class SpeedServo{
    virtual void set_speed_directly(const real_t rad) = 0;
public:
    void set_speed(const real_t rad){
        set_speed_directly(rad);    
    }

    virtual real_t get_speed() = 0;
};

};
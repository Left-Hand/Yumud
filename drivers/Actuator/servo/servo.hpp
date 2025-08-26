#pragma once

#include "core/math/real.hpp"
#include "types/regions/range2.hpp"

namespace ymd::drivers{



class ServoBase{
private:
    Range2<real_t> angle_range_ = {0, real_t(PI)};
protected:
    
    virtual void set_global_angle(const real_t rad) = 0;
    virtual real_t get_global_angle() = 0;
public:
    void set_range(const Range2<real_t> & range){
        angle_range_ = range;
    }
    
    void set_angle(const real_t rad){
        set_global_angle(real_t(PI) - angle_range_.clamp(rad));
    }

    real_t get_angle(){
        return get_global_angle();
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
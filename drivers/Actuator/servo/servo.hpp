#pragma once

#include "core/math/real.hpp"
#include "algebra/regions/range2.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "primitive/arithmetic/angle_range.hpp"

namespace ymd::drivers{



class ServoBase{
private:
    AngularRange<real_t> angle_range_ = AngularRange<real_t>::STRAIGHT;
protected:
    
    virtual void set_global_angle(const Angular<real_t> angle) = 0;
    virtual Angular<real_t> get_global_angle() = 0;
public:
    void set_range(const AngularRange<real_t> & range){
        angle_range_ = range;
    }
    
    void set_angle(const Angular<real_t> angle){
        TODO();
        // set_global_angle(Angular<real_t>::HALF - angle_range_.clamp(angle));
    }

    Angular<real_t> get_angle(){
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
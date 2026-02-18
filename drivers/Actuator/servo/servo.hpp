#pragma once

#include "core/math/real.hpp"
#include "algebra/regions/range2.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "primitive/arithmetic/angle_range.hpp"

namespace ymd::drivers{



class ServoBase{
private:
    AngularRange<iq16> angle_range_ = AngularRange<iq16>::STRAIGHT;
protected:
    
    virtual void set_global_angle(const Angular<iq16> angle) = 0;
    virtual Angular<iq16> get_global_angle() = 0;
public:
    void set_range(const AngularRange<iq16> & range){
        angle_range_ = range;
    }
    
    void set_angle(const Angular<iq16> angle){
        TODO();
        // set_global_angle(Angular<iq16>::HALF - angle_range_.clamp(angle));
    }

    Angular<iq16> get_angle(){
        return get_global_angle();
    }

};


class SpeedServo{
    virtual void set_speed_directly(const iq16 rad) = 0;
public:
    void set_speed(const iq16 rad){
        set_speed_directly(rad);    
    }

    virtual iq16 get_speed() = 0;
};

};
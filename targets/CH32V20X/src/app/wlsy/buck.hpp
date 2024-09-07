#ifndef __BUCK_HPP__

#define __BUCK_HPP__

#include "wlsy_inc.hpp"

namespace WLSY{

using Range = Range_t<real_t>;

class Buck{
protected:
    INA226 & ina;
    TimerOutChannelPosOnChip & pwm;

    struct PowerController{



        real_t current_range;

        real_t update(const ){

        }
    }

    struct CurrentController{
    public:
        real_t current = 0.0;



        real_t kp = 1.0;
        // real_t ki = 0.0;
    
        // real_t intergal = 0.0;
        // real_t intergal_clamp = 0.0;

        real_t output = 0.0;
        Range output_range;
    
        real_t update(const real_t targ_current, const real_t & real_current){
            auto error = targ_current - real_current;
            real_t kp_contribute = error * kp;

            real_t delta = kp_contribute;

            output += delta;
            output = output_range.clamp(output);

            return output;
        }
    };
public:
    Buck(INA226 & _ina, TimerOutChannelOnChip & _pwm):ina(_ina), pwm(_pwm){;}
    void init(){
        pwm.init();
    }

    void run(){

    }
}

}
#endif
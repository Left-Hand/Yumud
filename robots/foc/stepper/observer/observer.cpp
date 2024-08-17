#include "observer.hpp"
#include "../stepper.hpp"

void SpeedEstimator::Config::reset(){
    err_threshold = inv_poles / 16;
    max_cycles = foc_freq / 80;
}

real_t SpeedEstimator::update(const real_t position){
    real_t delta_pos = position - vars.last_position;
    real_t abs_delta_pos = ABS(delta_pos);
    real_t this_speed = (delta_pos * int(foc_freq) / int(vars.cycles));

    if(abs_delta_pos > err_threshold){
    
        vars.cycles = 1;
        vars.last_position = position;
        return vars.last_speed = this_speed;
    }else{
        vars.cycles++;
        if(vars.cycles > max_cycles){
            
            vars.cycles = 1;
            vars.last_position = position;
            return vars.last_speed = this_speed;
        }
    }

    return vars.last_speed;
}
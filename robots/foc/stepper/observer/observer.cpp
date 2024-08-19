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

    if(abs_delta_pos > config.err_threshold){
    
        vars.cycles = 1;
        vars.last_position = position;
        return vars.last_speed = this_speed;
    }else{
        vars.cycles++;
        if(vars.cycles > config.max_cycles){
            
            vars.cycles = 1;
            vars.last_position = position;
            return vars.last_speed = this_speed;
        }
    }

    return vars.last_speed;
}

void InverseObserver::count(){

}


void OverrunObserver::count(){

}

void OverTempObserver::count(){

}

void StallOberserver::count(){

}

// struct StallObserver{
//     real_t max_position_error;
//     real_t stall_speed_threshold;
//     uint32_t entry_time_ms;
//     uint32_t timeout_ms;

//     static constexpr uint32_t no_stall_time_ms = 0;

//     bool update(const real_t target_position, const real_t measured_position, const real_t measured_speed){
//         auto current_time_ms = millis();
//         if(abs(measured_speed) < stall_speed_threshold){ // stall means low speed
//             if(entry_time_ms == no_stall_time_ms){
//                 if(abs(target_position - measured_position) > max_position_error){ //
//                     entry_time_ms = current_time_ms;
//                 }
//             }else{
//                 return (current_time_ms - entry_time_ms> timeout_ms);
//             }
//         }else{
//             entry_time_ms = no_stall_time_ms;
//         }
//         return false;
//     }
// };

// class FOCStepper;

// struct ShutdownFlag{
// protected:
//     FOCStepper & m_stp;
//     bool state = false;

// public:
//     ShutdownFlag(FOCStepper & stp):m_stp(stp){;}

//     ShutdownFlag & operator = (const bool _state);
//     operator bool() const{
//         return state;
//     }
// };

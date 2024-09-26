#include "observer.hpp"
#include "../stepper.hpp"

void SpeedEstimator::Config::reset(){
    err_threshold = inv_poles / 16;
    max_cycles = foc_freq >> 7;
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

//     scexpr uint32_t no_stall_time_ms = 0;

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

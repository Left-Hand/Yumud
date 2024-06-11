#ifndef __STEPPER_OBS_HPP__

#define __STEPPER_OBS_HPP__

#include "constants.hpp"

    struct StallObserver{
        real_t max_position_error;
        real_t stall_speed_threshold;
        uint32_t entry_time_ms;
        uint32_t timeout_ms;

        static constexpr uint32_t no_stall_time_ms = 0;

        bool update(const real_t & target_position, const real_t & measured_position, const real_t & measured_speed){
            auto current_time_ms = millis();
            if(abs(measured_speed) < stall_speed_threshold){ // stall means low speed
                if(entry_time_ms == no_stall_time_ms){
                    if(abs(target_position - measured_position) > max_position_error){ //
                        entry_time_ms = current_time_ms;
                    }
                }else{
                    return (current_time_ms - entry_time_ms> timeout_ms);
                }
            }else{
                entry_time_ms = no_stall_time_ms;
            }
            return false;
        }
    };
#endif
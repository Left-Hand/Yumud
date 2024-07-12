#pragma once

#include "../constants.hpp"

struct SpeedEstimator{
public:
    struct Config{
        real_t err_threshold = inv_poles/4;
        size_t max_cycles = foc_freq / 40;//att least 40Hz
    };

    Config & config;
protected:
    const real_t & err_threshold = config.err_threshold;
    const size_t & max_cycles = config.max_cycles;

    real_t last_position = 0;
    real_t last_speed = 0;
    size_t cycles = 1;
public:
    SpeedEstimator(Config & _config):config(_config){}

    void reset(){
        // *this = SpeedEstimator(config);
        last_position = 0;
        last_speed = 0;
        cycles = 1;
    }

    real_t update(const real_t position){
        real_t delta_pos = position - last_position;
        real_t abs_delta_pos = ABS(delta_pos);
        real_t this_speed = (delta_pos * int(foc_freq) / int(cycles));

        if(abs_delta_pos > err_threshold){//high speed one cycle
            // real_t delta_speed_per_cycle_clamp = ABS(last_speed / (cycles * 8));
            // delta_speed_per_cycle = SIGN_AS(MIN(ABS(this_speed - last_speed)/cycles, delta_speed_per_cycle_clamp), this_speed - last_speed);

            cycles = 1;
            last_position = position;
            return last_speed = this_speed;
        }else{
            cycles++;
            if(cycles > max_cycles){
                
                cycles = 1;
                last_position = position;
                // delta_speed_per_cycle = 0;
                return last_speed = this_speed;
            }
        }

        // if(ABS(last_speed) < (1)) return last_speed + delta_speed_per_cycle * cycles;
        return last_speed;
    }
};

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

class Stepper;

struct ShutdownFlag{
protected:
    Stepper & m_stp;
    bool state = false;

public:
    ShutdownFlag(Stepper & stp):m_stp(stp){;}

    ShutdownFlag & operator = (const bool _state);
    operator bool() const{
        return state;
    }
};

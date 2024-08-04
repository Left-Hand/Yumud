#pragma once

#include "../constants.hpp"

struct SpeedEstimator{
public:
    struct Config{
        real_t err_threshold = inv_poles/8;
        size_t max_cycles = foc_freq / 40;//at least 40Hz
    };

    Config & config;
protected:
    const real_t & err_threshold = config.err_threshold;
    const size_t & max_cycles = config.max_cycles;

    struct Vars{
        real_t last_position;
        real_t last_speed;
        size_t cycles;
        void reset(){
            last_position = 0;
            last_speed = 0;
            cycles = 1;
        }
    };

    Vars vars;
public:
    SpeedEstimator(Config & _config):config(_config){}

    void reset(){
        vars.reset();
    }


    real_t update(const real_t position){
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
};


class IntegralableObserverConcept{
public:
    struct Config{
        size_t error_grade;
        size_t active_threshold;

        void reset(){
            error_grade = 3;
            active_threshold = 10;
        }

    };

    Config & config;
    size_t grade = 0;

    IntegralableObserverConcept(Config & _config):config(_config){}

    virtual void count() = 0;
    void reset(){
        grade = 0;
    }

    operator bool() const {
        return grade > config.active_threshold;
    }
};

class InverseObserver:public IntegralableObserverConcept{
    void update();
};

class OverrunObserver{
    void update();
};

class StallOberserver{

};
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

// class Stepper;

// struct ShutdownFlag{
// protected:
//     Stepper & m_stp;
//     bool state = false;

// public:
//     ShutdownFlag(Stepper & stp):m_stp(stp){;}

//     ShutdownFlag & operator = (const bool _state);
//     operator bool() const{
//         return state;
//     }
// };

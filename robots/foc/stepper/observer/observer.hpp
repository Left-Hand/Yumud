#pragma once

#include "robots/foc/motor_utils.hpp"


namespace yumud::foc{
struct SpeedEstimator{
public:
    struct Config{
        real_t err_threshold = inv_poles / 4;
        size_t est_freq = foc_freq;
        size_t max_cycles = foc_freq >> 7;
    };

    const Config & config;
protected:

    struct Vars{
        real_t last_position;
        real_t last_raw_speed;
        real_t last_speed;
        size_t cycles;
        void reset(){
            last_position = 0;
            last_raw_speed = 0;
            last_speed = 0;
            cycles = 1;
        }
    };

    Vars vars;


    real_t update_raw(const real_t position);
public:
    SpeedEstimator(const Config & _config):config(_config){
        reset();
    }

    void reset(){
        vars.reset();
    }

    real_t update(const real_t position){
        auto this_spd = update_raw(position);
        return vars.last_speed = (vars.last_speed * 127 + this_spd) >> 7;
        // return vars.last_speed = (vars.last_speed * 255 + this_spd) >> 8;
        // return vars.last_speed = (vars.last_speed * 31 + this_spd) >> 5;
    }
    real_t get() const {return vars.last_speed;} 
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
    void count() override;
};

class OverrunObserver:public IntegralableObserverConcept{
    void count() override;
};

class OverTempObserver:public IntegralableObserverConcept{
    void count() override;

};

class StallOberserver:public IntegralableObserverConcept{
    void count() override;

};


}
#pragma once

#include "../motor_utils.hpp"

struct SpeedEstimator{
public:
    struct Config{
        real_t err_threshold = inv_poles / 16;
        size_t est_freq = foc_freq;
        size_t max_cycles = foc_freq >> 7;
    };

    Config & config;
protected:

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
    SpeedEstimator(Config & _config):config(_config){
        reset();
    }

    void reset(){
        vars.reset();
    }


    __fast_inline real_t update(const real_t position);

    __fast_inline real_t get() const {return vars.last_speed;} 
};

real_t SpeedEstimator::update(const real_t position){
    real_t delta_pos = position - vars.last_position;
    real_t abs_delta_pos = ABS(delta_pos);
    real_t this_speed = (delta_pos * int(config.est_freq) / int(vars.cycles));

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

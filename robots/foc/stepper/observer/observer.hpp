#pragma once

#include "../motor_utils.hpp"

struct SpeedEstimator{
public:
    struct Config{
        real_t err_threshold;
        size_t max_cycles;

        void reset();
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
        config.reset();
        vars.reset();
    }


    real_t update(const real_t position);
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

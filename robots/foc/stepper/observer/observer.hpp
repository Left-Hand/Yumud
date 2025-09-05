#pragma once

#include "robots/foc/motor_utils.hpp"


namespace ymd::foc{

class IntegralableObserverIntf{
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

    IntegralableObserverIntf(Config & _config):config(_config){}

    virtual void count() = 0;
    void reset(){
        grade = 0;
    }

    operator bool() const {
        return grade > config.active_threshold;
    }
};

class InverseObserver:public IntegralableObserverIntf{
    void count() override;
};

class OverrunObserver:public IntegralableObserverIntf{
    void count() override;
};

class OverTempObserver:public IntegralableObserverIntf{
    void count() override;

};

class StallOberserver:public IntegralableObserverIntf{
    void count() override;

};


}
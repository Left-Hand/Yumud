#pragma once

#include "sys/math/real.hpp"


namespace ymd::foc{

class FaulterIntf{
public:
    virtual bool isFaulted() = 0;
};

class CurrentLimiterIntf:public FaulterIntf{
public:
    // virtual void update
    virtual real_t getMaxCurrent() = 0;
    virtual real_t getMinCurrent() = 0;
};



class CurrentLimiterByThermistor:public CurrentLimiterIntf{
public:
    struct Config{
        real_t crisis_temp;
        real_t max_temp;
        real_t max_curr;
    };
protected:
    const Config & config;
public:
    CurrentLimiterByThermistor(const Config & config):
        config(config){;}
    
    bool isFaulted(){

    }
};

class CurrentLimiterByPower:public CurrentLimiterIntf{
public:
    struct Config{
        real_t crisis_current;
        real_t gain;
        real_t threshold;
    };
    const Config & config;
public:
    CurrentLimiterByPower(const Config & config):
        config(config){;}
    
    bool isFaulted(){

    }
};

class CurrentLimiterBySpeed:public CurrentLimiterIntf{

};


}
#pragma once

#include "sys/math/real.hpp"

namespace ymd::foc{

class Pll{
public:
    struct Config{
        real_t kp;
        real_t ki;
        real_t period;
    };
protected:
    const Config & config_;
    real_t theta_;
    real_t omega_;
public: 
    Pll(Config & config) :
        config_(config){;}

    void update(const real_t theta){
        const real_t theta_err = theta - theta_;
        omega_ = config_.kp * theta_err;
    }
}
}
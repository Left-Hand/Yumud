#pragma once

#include "sys/math/real.hpp"

class PIController{
public:
    struct Config{
        real_t kp;
        real_t ki;
        real_t out_min;
        real_t out_max;
    };
protected:
    const Config config_;
    // const real_t inv_ki;
    real_t i_out_;
    // real_t int_;
    real_t output_;
public:
    PIController(const Config & config): 
        config_(config)
        // inv_ki(1/config.ki)
        {;}

    real_t update(const real_t targ, const real_t meas){
        const real_t err = targ - meas;

        const real_t p_out = config_.kp * err;
        if(unlikely(p_out >= config_.out_max)){
            // int_ = 0;
            i_out_ = 0;
            return output_ = config_.out_max;
        }else if(unlikely(p_out <= config_.out_min)){
            // int_ = 0;
            i_out_ = 0;
            return output_ = config_.out_min;
        }else{
            // int_ = CLAMP(int_ + err, (config_.out_min - p_out) * inv_ki, (config_.out_max - p_out) * inv_ki);
            i_out_ = CLAMP(i_out_ + err * config_.ki, (config_.out_min - p_out), (config_.out_max - p_out));
            // return output_ = CLAMP(p_out + int_ * config_.ki, config_.out_min, config_.out_max);
            // return output_ = CLAMP(p_out + i_out_ , config_.out_min, config_.out_max);
            return output_ = p_out + i_out_;
        }
        // return output_;
    }

    real_t output() const {
        return output_;
    }
};

class IController{
public:
    struct Config{
        real_t ki;
        real_t out_min;
        real_t out_max;
    };
protected:
    const Config config_;
    real_t output_;
public:
    IController(const Config & config): 
        config_(config)
        {;}

    real_t update(const real_t targ, const real_t meas){
        const real_t err = targ - meas;

        return output_ = CLAMP(output_ + err * config_.ki, (config_.out_min), (config_.out_max));
    }

    real_t output() const {
        return output_;
    }
};
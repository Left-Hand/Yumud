#pragma once

#include "core/math/iq/iq_t.hpp"

namespace ymd::dsp{


class DeltaPdController{
public:
    struct Config{
        q20 kp;
        q20 kd;
        q20 out_min;
        q20 out_max;
        unsigned int fs;
    };

public:
    DeltaPdController(const Config & cfg){
        reconf(cfg);
        reset();    
    }

    void reset(){
        i_out_ = 0;
        output_ = out_min_;
    }
    void reconf(const Config & cfg){
        kp_ = cfg.kp;
        kd_ = cfg.kd;
        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;
        inv_fs_ = 1_q24 / cfg.fs;
    }

    // static q20 calc_forward_feedback(const q20 targ){
    //     return sqrt((targ + 5.3_q24) * q20(1.0 / 18));
    // }
    
    void update(const q20 targ, const q20 meas){
        const auto x1 = targ - meas;
        const auto x2 = x1 - last_err;
        last_err = x1;

        output_ = CLAMP(output_ + (x1 * kp_) * inv_fs_ + x2 * kd_, out_min_, out_max_);

    }

    const auto & get() const {
        return output_;
    }
private:
    q20 kp_;
    q20 kd_;
    q20 inv_fs_;
    q20 out_min_;
    q20 out_max_;

    q20 i_out_;
    q20 output_;

    q20 last_err;
};

class DeltaPController{
public:
    struct Config{
        q20 kp;
        q20 out_min;
        q20 out_max;
        unsigned int fs;
    };

public:
    DeltaPController(const Config & cfg){
        reconf(cfg);
        reset();    
    }

    void reset(){
        output_ = out_min_;
    }
    void reconf(const Config & cfg){
        kp_ = cfg.kp;
        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;
        inv_fs_ = 1_q20 / cfg.fs;
    }
    
    void update(const q20 targ, const q20 meas){
        const auto x1 = targ - meas;
        last_err = x1;

        output_ = CLAMP(output_ + (x1 * kp_) * inv_fs_, out_min_, out_max_);
    }

    const auto & get() const {
        return output_;
    }
private:
    q20 kp_;
    q20 inv_fs_;
    q20 out_min_;
    q20 out_max_;

    q20 output_;

    q20 last_err;
};

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

}
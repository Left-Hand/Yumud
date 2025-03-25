#pragma once

#include "core/utils/immutable.hpp"
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


}
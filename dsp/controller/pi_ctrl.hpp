#pragma once

#include "core/utils/immutable.hpp"
#include "core/math/iq/iq_t.hpp"

namespace ymd::dsp{


class myPIController{
public:
    struct Config{
        q20 kp;
        q20 ki;
        q20 out_min;
        q20 out_max;
        unsigned int fs;
    };
protected:
    q20 kp_;
    q20 ki_by_fs_;
    q20 out_min_;
    q20 out_max_;

    q20 i_out_;
    q20 output_;
public:
    myPIController(const Config & cfg){
        reconf(cfg);
        reset();    
    }

    void reset(){
        i_out_ = 0;
        output_ = out_min_;
    }
    void reconf(const Config & cfg){
        kp_ = cfg.kp;
        ki_by_fs_ = cfg.ki / cfg.fs;
        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;
    }

    // static q20 calc_forward_feedback(const q20 targ){
    //     return sqrt((targ + 5.3_q24) * q20(1.0 / 18));
    // }
    
    void update(const q20 targ, const q20 meas){
        const q20 err = targ - meas;

        // const q20 p_out = kp_ * err;

        // if(unlikely(p_out >= out_max_)){
        //     i_out_ = 0;
        //     output_ = out_max_;
        //     return;
        // }else if(unlikely(p_out <= out_min_)){
        //     i_out_ = 0;
        //     output_ = out_min_;
        //     return;
        // }else{
            // i_out_ = CLAMP(i_out_ + err * ki_by_fs_, (out_min_ - p_out), (out_max_- p_out));
            i_out_ = CLAMP(i_out_ + err * ki_by_fs_, -1, 1);
            // output_ = CLAMP(p_out + i_out_, out_min_, out_max_);
            output_ = CLAMP(i_out_ + output_, out_min_, out_max_);
            // output_ = CLAMP(i_out_, out_min_, out_max_);
            return;
        // }
    }

    const auto & get() const {
        return output_;
    }
};


}
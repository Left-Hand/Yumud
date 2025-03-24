#pragma once

#include "core/utils/immutable.hpp"
#include "core/math/iq/iq_t.hpp"

namespace ymd::dsp{

class IController{
public:
    struct Config{
        q20 ki;
        q20 out_min;
        q20 out_max;
        unsigned int fs;
    };
protected:
    q20 ki_by_fs_;
    q20 out_min_;
    q20 out_max_;
    q20 output_;
public:
    IController(const Config & cfg){
        reconf(cfg);
        reset();
    }


    void reset(){
        output_ = out_min_;
    }

    void reconf(const Config & cfg){
        ki_by_fs_ = cfg.ki / cfg.fs;
        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;
    }

    void update(const q20 targ, const q20 meas){
        const q20 err = targ - meas;

        const auto temp_output = output_ + ki_by_fs_ * err;
        output_ = CLAMP(temp_output, out_min_, out_max_);
    }

    q20 output() const {
        return output_;
    }
};

        
}
#pragma once

#include "core/utils/immutable.hpp"
#include "core/math/iq/iq_t.hpp"

namespace ymd::dsp{

class SlidingModeController {
public:
    struct Config{
        q20 c;
        q20 q;

        q20 out_min;
        q20 out_max;

        unsigned int fs;

    };
    SlidingModeController(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        c_.borrow_mut() = cfg.c;
        q_.borrow_mut() = cfg.q;

        out_min_.borrow_mut() = cfg.out_min;
        out_max_.borrow_mut() = cfg.out_max;

        fs_.borrow_mut() = cfg.fs;
    }

    void reset(){
        output_.borrow_mut() = out_min_.get();
    }

    void update(const q20 targ,const q20 meas) {
        const auto c = c_.get();
        const auto q = q_.get();
        // const auto fs = fs_.get();

        const q20 x1 = targ - meas;
        const q20 x2 = (x1 - err_prev_.get());
        err_prev_.borrow_mut() = x1;

        const q20 s = c * x1 + x2;
        const q20 delta = c * x2 + q * s;

        output_.borrow_mut() = CLAMP(output_.get() + delta, out_min_.get(), out_max_.get());
    }

    auto get() const {return output_.get();}

private:

    immutable_t<q20> c_ = 0;
    immutable_t<q20> q_ = 0;
    
    immutable_t<q20> out_min_ = 0;
    immutable_t<q20> out_max_ = 0;

    immutable_t<unsigned int> fs_ = 0;

    immutable_t<q20> output_  = 0;

    immutable_t<q20> err_prev_ = 0;
};
}
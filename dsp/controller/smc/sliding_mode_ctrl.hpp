#pragma once

#include "core/math/iq/iq_t.hpp"

namespace ymd::dsp{

class SlideModeController {
public:
    struct Config{
        q20 c;
        q20 q;

        q20 out_min;
        q20 out_max;

        uint32_t fs;

    };
    explicit constexpr SlideModeController(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        c_ = cfg.c;
        q_ = cfg.q;

        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;

        fs_ = cfg.fs;
    }

    constexpr void reset(){
        output_ = out_min_;
    }

    constexpr void update(const q20 targ,const q20 meas) {
        const auto c = c_;
        const auto q = q_;
        // const auto fs = fs_;

        const q20 x1 = targ - meas;
        const q20 x2 = (x1 - err_prev_);
        err_prev_ = x1;

        const q20 s = c * x1 + x2;
        const q20 delta = c * x2 + q * s;

        output_ = CLAMP(output_ + delta, out_min_, out_max_);
    }

    constexpr auto get() const {return output_;}

private:

    q20 c_ = 0;
    q20 q_ = 0;
    
    q20 out_min_ = 0;
    q20 out_max_ = 0;

    uint32_t fs_ = 0;

    q20 output_  = 0;

    q20 err_prev_ = 0;
};
}
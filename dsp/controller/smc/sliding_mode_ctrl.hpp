#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::dsp{

struct [[nodiscard]] SlideModeController {
public:
    struct [[nodiscard]] Config{
        iq20 c;
        iq20 q;

        iq20 out_min;
        iq20 out_max;

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

    constexpr void update(const iq20 targ,const iq20 meas) {
        const auto c = c_;
        const auto q = q_;
        // const auto fs = fs_;

        const iq20 x1 = targ - meas;
        const iq20 x2 = (x1 - err_prev_);
        err_prev_ = x1;

        const iq20 s = c * x1 + x2;
        const iq20 delta = c * x2 + q * s;

        output_ = CLAMP(output_ + delta, out_min_, out_max_);
    }

    constexpr auto output() const {return output_;}

private:

    iq20 c_ = 0;
    iq20 q_ = 0;
    
    iq20 out_min_ = 0;
    iq20 out_max_ = 0;

    uint32_t fs_ = 0;

    iq20 output_  = 0;

    iq20 err_prev_ = 0;
};
}
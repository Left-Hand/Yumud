#pragma once

#include "core/math/iq/fixed_t.hpp"

namespace ymd::dsp{

class Integrator{
public:
    struct Config{
        uint32_t fs;
        iq20 ki;
        iq20 out_min;
        iq20 out_max;
    };
    constexpr Integrator(const Config & cfg){
        reconf(cfg);
        reset();
    }
    constexpr void reset(){
        output_ = out_min_;
    }

    constexpr void reconf(const Config & cfg){
        ki_by_fs_ = cfg.ki / cfg.fs;
        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;
    }

    constexpr void update(const iq20 err){

        const auto temp_output = output_ + ki_by_fs_ * err;
        output_ = CLAMP(temp_output, out_min_, out_max_);
    }

    [[nodiscard]] constexpr iq20 output() const {
        return output_;
    }

    [[nodiscard]] constexpr iq20 operator ()(const iq20 err){
        update(err);
        return output_;
    }

private:
    iq20 ki_by_fs_;
    iq20 out_min_;
    iq20 out_max_;
    iq20 output_;
};

        
}
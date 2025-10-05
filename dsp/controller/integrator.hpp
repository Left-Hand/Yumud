#pragma once

#include "core/math/iq/iq_t.hpp"

namespace ymd::dsp{

class Integrator{
public:
    struct Config{
        uint32_t fs;
        q20 ki;
        q20 out_min;
        q20 out_max;
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

    constexpr void update(const q20 err){

        const auto temp_output = output_ + ki_by_fs_ * err;
        output_ = CLAMP(temp_output, out_min_, out_max_);
    }

    [[nodiscard]] constexpr q20 output() const {
        return output_;
    }

    [[nodiscard]] constexpr q20 operator ()(const q20 err){
        update(err);
        return output_;
    }

private:
    q20 ki_by_fs_;
    q20 out_min_;
    q20 out_max_;
    q20 output_;
};

        
}
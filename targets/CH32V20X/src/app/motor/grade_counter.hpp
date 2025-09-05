#pragma once

#include "core/platform.hpp"


namespace ymd::foc{

struct GradeCounter{
public:
    struct Config{
        uint step;
        uint threshold;
    };
    GradeCounter(const Config & config):
        step_(config.step),
        threshold_(config.threshold),
        grades_(0){;}

    void reconf(const Config & config){
        step_ = config.step;
        threshold_ = config.threshold;
    }

    __fast_inline void reset(){
        grades_ = 0;
    }

    __fast_inline void update(bool match){
        if(unlikely(match)) grades_ += step_;
        else grades_ = CLAMP(grades_ - 1, 0, 2 * threshold_);
    }

    __fast_inline bool overflow() const{
        return grades_ >= threshold_;
    }

    __fast_inline auto grades() const{
        return grades_;
    }

private:
    uint step_;
    uint threshold_;
    uint grades_;
};

}
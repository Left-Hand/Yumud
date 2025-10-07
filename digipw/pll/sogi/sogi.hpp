#pragma once

#include "core/math/real.hpp"

namespace ymd::foc{
class SogiF{
public:

    struct Config{
        float kp;
        float w;
        float dt;
    };

    const Config config_;

    float u_alpha_;
    float u_beta_;


    constexpr SogiF(const Config& config): config_(config){;}

    constexpr void reset(){
        u_alpha_ = 0.0f;
        u_beta_ = 0.0f;
    }

    constexpr void update(const float uin){
        const auto u_alpha = u_alpha_;
        const auto u_beta = u_beta_;

        u_alpha_ += ((uin - u_alpha) * config_.kp - u_beta) * config_.w * config_.dt;
        u_beta_ += u_alpha * config_.w * config_.dt;
    }
};

class SogiQ{
public:

    struct Config{
        uint32_t fs;
        q16 w;
    };


    constexpr SogiQ(const Config & config):
        w_by_freq(config.w / config.fs){
        reset();
    }

    constexpr void reset(){
        u_alpha_ = 0;
        u_beta_ = 0;
    }

    constexpr void update(const q16 uin){
        static constexpr q16 kp = q16(1.414_r);
        const auto u_alpha = u_alpha_;
        const auto u_beta = u_beta_;

        u_alpha_ += (((uin)- u_alpha) * kp - u_beta) * w_by_freq;
        u_beta_ += u_alpha * w_by_freq;
    }

    constexpr AbCurrent ab() const {
        return {u_alpha_, u_beta_};
    }

private:
    const q16 w_by_freq;
    q16 u_alpha_;
    q16 u_beta_;

};
}
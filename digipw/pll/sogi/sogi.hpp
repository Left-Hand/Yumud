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


    SogiF(const Config& config): config_(config){;}

    void reset(){
        u_alpha_ = 0.0f;
        u_beta_ = 0.0f;
    }

    void update(const float uin){
        const auto u_alpha = u_alpha_;
        const auto u_beta = u_beta_;

        u_alpha_ += ((uin - u_alpha) * config_.kp - u_beta) * config_.w * config_.dt;
        u_beta_ += u_alpha * config_.w * config_.dt;
    }
};

class SogiQ{
public:

    struct Config{
        iq_t<16> w;
        int freq;
    };

    const iq_t<16> w_by_freq;
    iq_t<16> u_alpha_;
    iq_t<16> u_beta_;

    SogiQ(const Config & config):
        w_by_freq(config.w / config.freq){
        reset();
    }

    void reset(){
        u_alpha_ = 0;
        u_beta_ = 0;
    }

    void update(const iq_t<16> uin){
        scexpr iq_t<16> kp = iq_t<16>(1.414_r);
        const auto u_alpha = u_alpha_;
        const auto u_beta = u_beta_;
   
        u_alpha_ += (((uin)- u_alpha) * kp - u_beta) * w_by_freq;
        u_beta_ += u_alpha * w_by_freq;
    }

    AbCurrent ab() const {
        return {u_alpha_, u_beta_};
    }
};
}
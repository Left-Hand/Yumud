#pragma once
#include <cmath>

namespace ymd::dsp{

template<typename T>
class ExpSmoother {
public:
    constexpr void init(T sampleRate) {
        sampleRate_ = sampleRate;
    }

    constexpr T process(T in) {
        if (in > latch_) {
            latch_ = latch_ * biggerCoeff_ + (1 - biggerCoeff_) * in;
        }
        else {
            latch_ = latch_ * smallerCoeff_ + (1 - smallerCoeff_) * in;
        }
        return latch_;
    }

    constexpr void set_attack_time(T ms) {
        biggerCoeff_ = std::exp(-1.0f / (sampleRate_ * ms / 1000.0f));
    }

    constexpr void set_release_time(T ms) {
        smallerCoeff_ = std::exp(-1.0f / (sampleRate_ * ms / 1000.0f));
    }
private:
    T sampleRate_ = 0;
    T latch_ = 0;
    T biggerCoeff_ = 0;
    T smallerCoeff_ = 0;
};

}
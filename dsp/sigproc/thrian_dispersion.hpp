#pragma once
#include <cstdint>
#include "algebra/points/complex/complex.hpp"

namespace ymd::dsp {

template<typename T>
class ThrianDispersion {
public:
    static constexpr uint32_t kMaxNumAPF = 4;
    static constexpr auto twopi = std::numbers::pi_v<T> * 2;

    ThrianDispersion(const )
    constexpr void init(T sampleRate) {
        sampleRate_ = sampleRate;
    }

    constexpr T process(T in) {
        for (uint32_t i = 0; i < kMaxNumAPF; ++i) {
            in = process_filter(in, i);
        }
        return in;
    }

    constexpr T get_phase_delay(T freq) const {
        const auto omega = twopi * freq / sampleRate_;
        const auto cpx = Complex<T>::from_polar(1.0f, omega);
        const auto up = b0_ * cpx * cpx + a1_ * cpx + b2_;
        const auto down = cpx * cpx + a1_ * cpx + a2_;
        const auto radius = -std::arg(up / down);
        if (radius < 0) {
        radius += twopi;
        }
        return radius * kMaxNumAPF / omega;
    }

    constexpr void set_group_delay(T delay) {
        if (delay < 1) {
            a1_ = 0;
            a2_ = 0;
            b0_ = 1;
            b2_ = 0;
        }else {
            a1_ = -2 * something(delay, 1);
            a2_ = something(delay, 2);
            b0_ = a2_;
            b2_ = 1.0f;
        }
    }

    constexpr void panic() {
        for (auto& s : latchs_) {
            s.latch1_ = 0;
            s.latch2_ = 0;
        }
    }

    constexpr Complex<T> get_response(T omega) const {
        const auto z = Complex<T>::from_polar(omega);
        const auto up = a2_ * z * z + a1_ * z + 1.0f;
        const auto down = z * z + a1_ * z + a2_;
        return up / down;
    }
private:
    constexpr T process_filter(T in, uint32_t i) {
        auto& latch1 = latchs_[i].latch1_;
        auto& latch2 = latchs_[i].latch2_;
        auto t = in - a1_ * latch1 - a2_ * latch2;
        // auto y = t * a2_ + a1_ * latch1 + latch2;
        auto y = t * b0_ + a1_ * latch1 + b2_ * latch2;
        latch2 = latch1;
        latch1 = t;
        return y;
    }


    static constexpr T something(T delay, int k) {
        T ret = 1;
        for (int n = 0; n <= 2; ++n) {
            ret *= (delay - 2 + n) / (delay - 2 + k + n);
        }
        return ret;
    }


    T sampleRate_{};
    T a2_{};
    T a1_{};
    T b0_{};
    T b2_{};

    struct APFData {
        T latch1_{};
        T latch2_{};
    };
    
    std::array<APFData, kMaxNumAPF>latchs_{};
};

} // namespace dsp

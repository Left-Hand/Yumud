#pragma once

#include <cstdint>


namespace ymd::dsp{

class LCGNoiseSiggen {
public:
    LCGNoiseSiggen(uint32_t seed = 0) : state_(seed) {}

    void setSeed(uint32_t seed) { state_ = seed; }

    // 生成0到2^32-1的无符号数
    constexpr __fast_inline
    uint32_t update() {
        state_ = (a_ * state_ + c_) & mask_;
        return state_;
    }

private:
    uint32_t state_;

    static constexpr uint32_t a_ = 1103515245; // Glibc参数
    static constexpr uint32_t c_ = 12345;
    static constexpr uint32_t mask_ = 0xFFFFFFFF; // 2^32-1
};

}
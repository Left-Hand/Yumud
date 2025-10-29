#pragma once

#include <cstdint>


namespace ymd::dsp{

class LcgNoiseSiggen {
public:
    constexpr explicit LcgNoiseSiggen(uint32_t seed = 0) : state_(seed) {}

    constexpr 
    void set_seed(uint32_t seed) { state_ = seed; }

    // 生成0到2^32-1的无符号数
    constexpr __fast_inline
    void update() {
        state_ = (a_ * state_ + c_) & mask_;
    }

    [[nodiscard]] constexpr __fast_inline
    uint32_t get() const { return state_; }

    [[nodiscard]] constexpr __fast_inline
    q16 get_as_01(){
        update();
        return q16::from_i32((get() & 0xffff));
    }
    
    [[nodiscard]] constexpr __fast_inline
    std::tuple<real_t, real_t> get_as_01x2(){
        update();
        const uint32_t temp = get();
        const uint32_t u0 = temp >> 16;
        const uint32_t u1 = temp & 0xffff;
        return {iq_t<16>::from_i32(u0), iq_t<16>::from_i32(u1)};
    }

private:
    uint32_t state_;

    static constexpr uint32_t a_ = 1103515245; // Glibc参数
    static constexpr uint32_t c_ = 12345;
    static constexpr uint32_t mask_ = 0xFFFFFFFF; // 2^32-1
};

}
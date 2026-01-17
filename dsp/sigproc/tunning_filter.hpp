#pragma once
#include <cstdint>

// 可实现小数群延迟的全通滤波器

namespace ymd::dsp {

// one pole all pass filter

template<typename T>
struct TunningFilter final{
public:
    struct Config{
        T delay;
    };

    constexpr explicit TunningFilter(const Config & cfg){
        reconf(cfg);
    }

    void reconf(const Config & cfg){
        set_delay(cfg.delay);
    }

    constexpr T operator ()(T in) {
        T v = latch_;
        T t = in - alpha_ * v;
        latch_ = t;
        return v + alpha_ * t;
    }

    constexpr void set_delay(T delay) { 
        alpha_ = delay_to_alpha(delay);
    }

    static constexpr T delay_to_alpha(T delay) noexcept{
        // thiran delay limit to 0.5 ~ 1.5
        if (delay < T(0.5f)) {
            return 0; // equal to one delay
        }else {
            T intergal_part = std::floor(delay);
            T fractional_part = delay - intergal_part;
            if (fractional_part < T(0.5f)) {
                fractional_part += T(1);
                intergal_part -= T(1);
            }
            return (T(1) - fractional_part) / (T(1) + fractional_part);
        }
    }
private:
    T latch_{};
    T alpha_{};
};

}
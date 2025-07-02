#pragma once

#include "delay.hpp"
// #include :

//可实现整数群延迟和小数群延迟的全通滤波器

namespace ymd::dsp{

template<typename T>
class CombAllpass {
public:
    constexpr CombAllpass(const std::span<T> pbuf):
        delay_line_(pbuf){;}

    constexpr T operator()(T in){
        auto v = delay_line_.get_last();
        auto t = in - alpha_ * v;
        delay_line_.push(t);
        last_ = v + alpha_ * t;
        return last_;
    }

    constexpr void  clear_internal(){delay_line_.clear_internal();}
    constexpr void  set_delay(T delay){
        const auto int_part = size_t(delay);
        const auto frac_part = delay - int_part;
        delay_line_.set_delay(int_part);
        alpha_ = map_delay_to_alpha(frac_part);
    }

    static constexpr T map_delay_to_alpha(T delay) noexcept{
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
    constexpr T delay() const { return delay_line_.delay(); }
    constexpr T alpha() const { return alpha_; }
    constexpr T last() const { return last_; }
private:
    DelayLine<T> delay_line_;
    T alpha_{0};
    T last_{0};
};

}
#pragma once

#include "delay.hpp"

//可实现整数群延迟和小数群延迟的全通滤波器

namespace ymd::dsp{

template<typename T>
class CombAllpass {
public:
    constexpr CombAllpass(const std::span<T> pbuf):
        delay_line_(pbuf){;}

    constexpr T process(T in){
        auto v = delay_line_.get_last();
        auto t = in - alpha_ * v;
        delay_line_.push(t);
        last_ = v + alpha_ * t;
        return last_;
    }

    constexpr void  clear_internal(){delay_line_.clear_internal();}
    constexpr void  set_delay(T delay){delay_line_.set_delay(delay);}
    constexpr T delay() const { return delay_line_.delay(); }
    constexpr void  set_alpha(T a){alpha_ = a;}
    constexpr T alpha() const { return alpha_; }
    constexpr T last() const { return last_; }
private:
    DelayLine delay_line_;
    T alpha_{0};
    T last_{0};
};

}
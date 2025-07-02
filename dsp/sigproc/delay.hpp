#pragma once
#include <array>
#include <cstdint>
#include "core/math/realmath.hpp"
namespace ymd::dsp {

template<typename T>
class DelayLine {
public:

    constexpr DelayLine(const std::span<T> pbuf):
        pbuf_(pbuf){;}

    constexpr T operator()(T in){
        write_pos_++;
        if(write_pos_ >= pbuf_.size()) write_pos_ = 0;
        pbuf_[write_pos_] = in;
        return get_last();
    }
    constexpr void push(T in){
        write_pos_++;
        if(write_pos_ >= pbuf_.size()) write_pos_ = 0;
        pbuf_[write_pos_] = in;
    }
    constexpr void clear_internal(){
        std::fill_n(pbuf_.begin(), pbuf_.size(), 0);
    }
    constexpr T get_last(){
        const auto read_idx = (write_pos_ + pbuf_.size() - delay_) % pbuf_.size();
        return pbuf_[read_idx];
    }
    constexpr void set_delay(size_t delay){
        delay_ = (delay);
    }
    constexpr T delay() const { 
        return delay_; 
    }
private:
    std::span<T> pbuf_{};
    size_t write_pos_ = 0;
    size_t delay_ = 0;

};

} // namespace dsp

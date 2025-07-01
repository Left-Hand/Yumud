#pragma once
#include <array>
#include <cstdint>

namespace ymd::dsp {

template<typename T>
class DelayLine {
public:

    constexpr DelayLine(std::span<T> & pbuf):
        pbuf_(pbuf){;}

    constexpr T process(T in){
        write_pos_++;
        write_pos_ &= kMask;
        pbuf_[write_pos_] = in;
        return get_last();
    }
    constexpr void push(T in){
        write_pos_++;
        write_pos_ &= kMask;
        pbuf_[write_pos_] = in;
    }
    constexpr void clear_internal(){
        std::fill_n(pbuf_.begin(), data.size(), 0);
    }
    constexpr T get_last(){
        const auto read_idx = write_pos_ + data_.size() - delay_;
        read_idx &= kMask;
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

    template<typename T>
    static constexpr my_clamp(T x, T min, T max){
        return x < min ? min : x > max ? max : x;
    }
};

} // namespace dsp

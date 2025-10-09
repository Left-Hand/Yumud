#pragma once

#include <cstdint>

namespace ymd::dsp{

template<typename T>
struct FirstOrderLowpassFilter{
public:
    struct Config {
        uint fc;
        uint fs;
    };

public:
    constexpr FirstOrderLowpassFilter(){
        reset();
    }

    constexpr FirstOrderLowpassFilter(const Config & config){
        reconf(config);
        reset();
    }

    constexpr void reconf(const Config & config){
        alpha_ = solve_alpha(config.fc, config.fs);
        wc_ = T(TAU) * config.fc;
    }

    constexpr void reset(){
        last_ = 0;
        last_x_ = 0;
        inited_ = false;
    }

    constexpr void update(const T x){
        if(unlikely(inited_ == false)){
            last_ = x;
            last_x_ = x;
            inited_ = true;
        }else{
            last_ = alpha_ * (last_ + x - last_x_);
            last_x_ = x;
        }

        // return last_ * wc_;
    }

    constexpr T get() const {
        return last_ * wc_;
    }

    static constexpr T solve_alpha(const auto fc, const auto fs){
        return (T(fs) / (fs + T(TAU) * fc));
    }

private:
    T alpha_;
    T wc_;

    T last_;
    T last_x_;
    bool inited_;
};

}
#pragma once

#include <cstdint>

namespace ymd::dsp{


template<typename T>
class FirstOrderHighpassFilter{
public:
    struct Config{
        T fc;
        uint fs;
    };


public:
    FirstOrderHighpassFilter() = default;
    constexpr FirstOrderHighpassFilter(const Config & config){
        reconf(config);
        reset();
    }

    constexpr void reconf(const Config & config){
        m_alpha = calculate_alaph(config.fc, config.fs);
    }

    constexpr void reset(){
        last = 0;
    }

    constexpr void update(const T x){
        last = m_alpha * (last + x - last_x);
        last_x = x;
    }

    constexpr T result() const{
        return last;
    }

    static constexpr T calculate_alaph(const T fc, const T fs){
        T omega_c = 2 * T(PI) * fc / fs;  // Angular cutoff frequency
        T alpha = 1 / (1 + omega_c);  // Alpha coefficient
        return alpha;
    }
private:
    T m_alpha;
    T last = 0;
    T last_x;
};


}

#pragma once

#include "core/math/real.hpp"

namespace ymd::dsp{

template<arithmetic T, size_t N>
class Order4ZeroPhaseShiftButterWothLowpassFilter{
public:
    struct Config{
        T fc;
    };

    void reconf(const Config & cfg){
        const T sr = N;
        const T fc = cfg.fc;

        wc = tan(fc*T(M_PI)/sr);
        k1 = T(1.414)*wc;
        k2 = wc*wc;
        a = k2/(1 + k1 + k2);
        b = 2*a;
        c = a;
        k3 = b/k2;
        d = - 2*a + k3;
        e = 1 - 2*a - k3;
    }

    void process(const std::span<const T, N> x){
        std::array<T, N> d1;
        d1[0] = x[0]; d1[1] = x[0];

        d1[N + 2] = x[N - 1]; d1[N + 3] = x[N - 1];
        for(size_t i = 0; i < N; i++){
            d1[i + 2] = x[i];
        }
        std::array<T, N> d2;
        d2[0] = x[0]; d2[1] = x[0];

        for(size_t i = 2; i < N + 2; i++){
            d2[i] = a*d1[i] + b*d1[i - 1] + c*d1[i - 2] +
                d*d2[i - 1] + e*d2[i - 2];
        }

        d2[N + 2] = d2[N + 1]; d2[N + 3] = d2[N + 1];
      
        y[N] = d2[N + 2]; y[N + 1] = d2[N + 3];
        for(int i = - (N - 1); i <= 0; i++){
            y[ - i] = a*d2[ - i + 2] + b*d2[ - i + 3] + c*d2[ - i + 4] +
                d*y[ - i + 1] + e*y[ - i + 2];
        }
    }

    std::span<const T, N> result() const {
        return std::span(y);
    }

    void reset(){
        y.fill(0);
    }
private:
    T wc, k1, k2, k3;
    T a, b, c, d, e;
    std::array<T, N> y;
};

template<arithmetic T, size_t N>
using O4ZpsBwLpf = Order4ZeroPhaseShiftButterWothLowpassFilter<T, N>;
}
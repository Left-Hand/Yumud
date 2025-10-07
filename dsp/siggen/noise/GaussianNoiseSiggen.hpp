#pragma once

#include "core/math/real.hpp"

namespace ymd::dsp{
template<arithmetic T>

struct GaussianNoiseGenerator{
public:
    T m_mu;
    T m_sigma;

    GaussianNoiseGenerator(T mu, T sigma):m_mu(mu),m_sigma(sigma){;}
    T update(){
        return gaussianNoise(m_mu, m_sigma);
    }
    static T gaussianNoise(T mu, T sigma){
        static T V1, V2, S;
        static int phase = 0;
        T X;
        T U1,U2;
        if ( phase == 0 ) {
            do {
                U1 = T(rand() % 8192) / 8192;
                U2 = T(rand() % 8192) / 8192;

                V1 = 2 * U1 - 1;
                V2 = 2 * U2 - 1;
                S = V1 * V1 + V2 * V2;
            } while(S >= 1 || S == 0);

            X = V1 * sqrt(-2 * log(S) / S);
        } else{
            X = V2 * sqrt(-2 * log(S) / S);
        }
        phase = 1 - phase;
        return mu+sigma*X;
    }
};

template<arithmetic T>
GaussianNoiseGenerator -> GaussianNoiseGenerator<T>;

}
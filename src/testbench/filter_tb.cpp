#include "tb.h"

#include "dsp/filter/BurrFilter.hpp"

#include <iterator>
#include <random>

template<typename real>
requires std::is_arithmetic_v<real>
struct GaussianNoiseGenerator_t{
public:
    real m_mu;
    real m_sigma;

    GaussianNoiseGenerator_t(real mu, real sigma):m_mu(mu),m_sigma(sigma){;}
    real update(){
        return gaussianNoise(m_mu, m_sigma);
    }
    static real gaussianNoise(real mu, real sigma){
        static real V1, V2, S;
        static int phase = 0;
        real X;
        real U1,U2;
        if ( phase == 0 ) {
            do {
                U1 = real(rand() % 8192) / 8192;
                U2 = real(rand() % 8192) / 8192;

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

using GaussianNoiseGenerator = GaussianNoiseGenerator_t<real_t>;


void filter_tb(OutputStream & logger){
    logger.setEps(4);

    real_t fi = 2;
    real_t fc = 2;
    real_t fs = 1000;

    const real_t mean = 0.0;//均值
    const real_t stddev = 1;//标准差
    // std::default_random_engine generator;
    // std::normal_distribution<real_t> dist(mean, stddev);

    GaussianNoiseGenerator gauss(mean, stddev);
    LowpassFilterZ_t<real_t> lpf{fc,fs};
    HighpassFilterZ_t<real_t> hpf{fc,fs};
    KalmanFilterZ_t<real_t> kmf{200,1};
    BurrFilter_t<real_t> brf;

    while(true){
        static uint32_t last_millis = millis();
        real_t omega = fi * TAU * t;

        real_t input;
        input += sin(omega);
        input += sin(21 * omega)/3;
        input += 0.1 * gauss.update();
        input += 3 * bool((millis() % 266) == 0);

        real_t lpf_output = lpf.update(input);
        real_t hpf_output = hpf.update(input);
        real_t kmf_output = kmf.update(input);
        real_t brf_output = brf.update(input);


        logger.println(input, lpf_output, hpf_output, kmf_output, brf_output);

        while(millis() == last_millis);
        last_millis = millis();
    }
}
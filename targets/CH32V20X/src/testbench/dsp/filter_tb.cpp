#include "src/testbench/tb.h"

#include "core/platform.h"
#include "core/math/real.hpp"

#include "dsp/filter/homebrew/BurrFilter.hpp"
#include "dsp/filter/rc/HighPassFilter.hpp"
#include "dsp/filter/rc/LowPassFilter.hpp"
#include "dsp/filter/kalman/KalmanFilter.hpp"

#include <iterator>




void filter_tb(OutputStream & logger){
    logger.setEps(4);

    // real_t fi = 2;
    // real_t fc = 2;
    // real_t fs = 1000;

    // const real_t mean = 0.0_r;//均值
    // const real_t stddev = 1.0_r;//标准差
    // // std::default_random_engine generator;
    // // std::normal_distribution<real_t> dist(mean, stddev);

    // GaussianNoiseGenerator gauss(mean, stddev);
    // LowpassFilterZ_t<real_t> lpf{fc,fs};
    // HighpassFilterZ_t<real_t> hpf{fc,fs};
    // KalmanFilterZ_t<real_t> kmf{200,1};
    // BurrFilter_t<real_t> brf;

    // while(true){
    //     static uint32_t last_millis = millis();
    //     real_t omega = fi * TAU * t;

    //     real_t input;
    //     input += sin(omega);
    //     input += sin(21 * omega)/3;
    //     input += 0.1 * gauss.update();
    //     input += 3 * bool((millis() % 266) == 0);

    //     real_t lpf_output = lpf.update(input);
    //     real_t hpf_output = hpf.update(input);
    //     real_t kmf_output = kmf.update(input);
    //     real_t brf_output = brf.update(input);


    //     logger.println(input, lpf_output, hpf_output, kmf_output, brf_output);

    //     while(millis() == last_millis);
    //     last_millis = millis();
    // }
}
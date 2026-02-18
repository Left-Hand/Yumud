#include "src/testbench/tb.h"

#include "core/platform.hpp"
#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"

#include <iterator>
#include "hal/bus/uart/hw_singleton.hpp"


using namespace ymd;

void filter_tb(OutputStream & logger){
    logger.set_eps(4);

    // iq16 fi = 2;
    // iq16 fc = 2;
    // iq16 fs = 1000;

    // const iq16 mean = 0.0_r;//均�?
    // const iq16 stddev = 1.0_r;//标准�?
    // // std::default_random_engine generator;
    // // std::normal_distribution<iq16> dist(mean, stddev);

    // GaussianNoiseGenerator gauss(mean, stddev);
    // LowpassFilterZ_t<iq16> lpf{fc,fs};
    // HighpassFilterZ_t<iq16> hpf{fc,fs};
    // KalmanFilterZ_t<iq16> kmf{200,1};
    // BurrFilter_t<iq16> brf;

    // while(true){
    //     static uint32_t last_millis = millis();
    //     iq16 omega = fi * TAU * t;

    //     iq16 input;
    //     input += sin(omega);
    //     input += sin(21 * omega)/3;
    //     input += 0.1 * gauss.update();
    //     input += 3 * bool((millis() % 266) == 0);

    //     iq16 lpf_output = lpf.update(input);
    //     iq16 hpf_output = hpf.update(input);
    //     iq16 kmf_output = kmf.update(input);
    //     iq16 brf_output = brf.update(input);


    //     logger.println(input, lpf_output, hpf_output, kmf_output, brf_output);

    //     while(millis() == last_millis);
    //     last_millis = millis();
    // }
}
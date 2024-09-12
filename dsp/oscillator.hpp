#ifndef __OSCILLATOR_HPP__

#define __OSCILLATOR_HPP__

template<typename real>
struct Osc{
    // real_t ms;

    real ang;
    real freq_khz;
    real amp;
    virtual real forward(const real & delta_ms) = 0;
};

#include "sys/math/real.hpp"

// template<typename real>
struct SineOsc:public Osc<real_t>{
    real_t forward(const real_t & delta_ms) override{
        ang += delta_ms * freq_khz;
        return sin(ang) * amp;
    }
};


#endif
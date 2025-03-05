#pragma once

#include "../lti.hpp"

template<arithmetic T>
class HighpassFilter_t{
public:
    const T m_alpha;
    T last = 0;
    T last_x;
public:
    HighpassFilter_t() = delete;

    HighpassFilter_t(const T fc, const T fs) : HighpassFilter_t(calculateAlaph(fc, fs)) {;}
    HighpassFilter_t(const T alpha) : m_alpha(alpha) {
        reset();
    }

    void reset(){
        last = 0;
    }

    T update(const T x){
        last = m_alpha * (last + x - last_x);
        last_x = x;
        return last;
    }

    static T calculateAlaph(const T fc, const T fs){
        T omega_c = 2 * T(PI) * fc / fs;  // Angular cutoff frequency
        T alpha = 1 / (1 + omega_c);  // Alpha coefficient
        return alpha;
    }
};

using HPFZ = HighpassFilter_t<real_t>;
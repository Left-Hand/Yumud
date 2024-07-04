#ifndef __HIGHPASS_FILTER_HPP__

#define __HIGHPASS_FILTER_HPP__

#include "../lti.hpp"


template<typename real, typename time>
class HighpassFilter_t:public Filter_t<real, time>{
protected:
    using Point = LtiUtils::Point<real, time>;

    Point last;
    const real inverse_tau;
    bool inited = false;
public:
    HighpassFilter_t(const real & cutoff_freq) : last(Point{real(0), time(0)}), inverse_tau(TAU * cutoff_freq) {;}

    real update(const real & x, const time & t) override {
        if(!inited){
            last.x = x;
            last.t = t;
            inited = true;
            return last.x;
        }
        real k = (t - last.t) * inverse_tau;
        last.x = k * x + (1 - k) * last.x;
        last.t = t;
        return x - last.x;
    }
};


template<typename real>
requires std::is_arithmetic_v<real>
class HighpassFilterZ_t{
public:
    const real m_alpha;
    real last = 0;
    real last_x;
public:
    HighpassFilterZ_t() = delete;

    HighpassFilterZ_t(const real fc, const real fs) : HighpassFilterZ_t(calculateAlaph(fc, fs)) {;}
    HighpassFilterZ_t(const real alpha) : m_alpha(alpha) {
        reset();
    }

    void reset(){
        last = 0;
    }

    real update(const real x){
        last = m_alpha * (last + x - last_x);
        last_x = x;
        return last;
    }

    static real calculateAlaph(const real fc, const real fs){
        real omega_c = 2 * M_PI * fc / fs;  // Angular cutoff frequency
        real alpha = 1 / (1 + omega_c);  // Alpha coefficient
        return alpha;
    }
};

using HPF = HighpassFilter_t<real_t, real_t>;
using HPFZ = HighpassFilterZ_t<real_t>;

#endif
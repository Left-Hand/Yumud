#ifndef __LOWPASS_FILTER_HPP__

#define __LOWPASS_FILTER_HPP__

#include "../lti.hpp"


template<typename real, typename time>
class LowpassFilter_t:public Filter_t<real, time>{
protected:
    using Point = LtiUtils::Point<real, time>;

    Point last;
    const real inverse_tau;
    bool inited = false;

    void init(const real & x){
        // if(!inited){
            last.x = x;
            inited = true;
            // return last.x;
        // }
    }
public:
    template<typename U>
    LowpassFilter_t(const U & cutoff_freq) : last(Point{real(0), time(0)}), inverse_tau(real(TAU * cutoff_freq)) {;}

    real update(const real & x, const time & t) override {
        if(!inited){
            init(x);
            last.t = t;
            return x;
        }
        real ret = forward(x, t - last.t);
        last.t = t;
        return ret;
    }

    real forward(const real & x, const time & delta){
        if(!inited){
            init(x);
            return x;
        }
        real b = delta * inverse_tau;
        real alpha = b / (b + 1);
        last.x = last.x + alpha * (x - last.x);
        return last.x;
    }
};

template<typename real>
class LowpassFilterZ_t{
public:
    const real m_alaph;
    real last = 0;
    bool inited = false;
public:
    LowpassFilterZ_t() = delete;

    LowpassFilterZ_t(const real fc, const real fs) : LowpassFilterZ_t(calculateAlaph(fc, fs)) {;}
    LowpassFilterZ_t(const real alaph) : m_alaph(alaph) {reset();}

    void reset(){
        last = 0;
    }

    real update(const real x){
        return last = last * m_alaph + (1-m_alaph) * x;
    }

    static real calculateAlaph(const real fc, const real fs){
        real omega_c = 2 * M_PI * fc / fs;  // Angular cutoff frequency
        real alpha = omega_c / (1 + omega_c);  // Alpha coefficient
        return alpha;
    }
};

using LPF = LowpassFilter_t<real_t, real_t>;
using LPFZ = LowpassFilterZ_t<real_t>;

#endif
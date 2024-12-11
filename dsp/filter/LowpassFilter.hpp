#pragma once

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
    LowpassFilter_t(const real & cutoff_freq) : last(Point{real(0), time(0)}), inverse_tau(real(TAU) * real(cutoff_freq)) {;}

    real update(const real & x, const time & tm) override {
        if(!inited){
            init(x);
            last.t = tm;
            return x;
        }
        real ret = forward(x, tm - last.t);
        last.t = tm;
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
requires std::is_arithmetic_v<real>
class LowpassFilterZ_t{
public:
    const real m_alaph;
    real last = 0;
public:
    LowpassFilterZ_t() = delete;

    LowpassFilterZ_t(const real fc, const real fs) : LowpassFilterZ_t(calculateAlaph(fc, fs)) {;}
    LowpassFilterZ_t(const real alaph) : m_alaph(alaph) {reset();}

    void reset(){
        last = 0;
    }

    real update(const real x){
        return last = last * (1-m_alaph) + (m_alaph) * x;
    }

    static real calculateAlaph(const real fc, const real fs){
        real omega_c = TAU * fc / fs;  // Angular cutoff frequency
        real alpha = omega_c / (1 + omega_c);  // Alpha coefficient
        return alpha;
    }
};

using LPF = LowpassFilter_t<real_t, real_t>;
using LPFZ = LowpassFilterZ_t<real_t>;